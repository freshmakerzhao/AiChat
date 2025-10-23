#include "NetworkManager.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
	: QObject(parent), manager(new QNetworkAccessManager(this)), currentReply(nullptr)
{
}

void NetworkManager::sendStreamingRequest(const QString &apiKey, const QString &apiUrl, 
										  const QJsonObject &requestData)
{
	// Cancel any ongoing request
	cancelCurrentRequest();

	// Create request
	QNetworkRequest request;
	request.setUrl(QUrl(apiUrl));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());
	request.setRawHeader("User-Agent", "Qt-AI-Chat/1.0");

	// Send request
	QJsonDocument doc(requestData);
	currentReply = manager->post(request, doc.toJson());
	
	// Connect signals
	connect(currentReply, &QNetworkReply::readyRead, this, &NetworkManager::onReadyRead);
	connect(currentReply, &QNetworkReply::finished, this, &NetworkManager::onFinished);
	connect(currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
			this, &NetworkManager::onError);
}

void NetworkManager::cancelCurrentRequest()
{
	if (currentReply) {
		currentReply->abort();
		currentReply->deleteLater();
		currentReply = nullptr;
		streamBuffer.clear();
	}
}

void NetworkManager::onReadyRead()
{
	if (!currentReply) return;

	// Read new data and append to buffer
	QByteArray newData = currentReply->readAll();
	streamBuffer.append(newData);

	// Process complete lines in buffer
	QStringList lines = QString::fromUtf8(streamBuffer).split('\n');
	
	// Keep the last potentially incomplete line in buffer
	if (!lines.isEmpty()) {
		streamBuffer = lines.takeLast().toUtf8();
	}

	// Process each complete line
	for (const QString &line : lines) {
		if (line.trimmed().isEmpty()) continue;
		
		// Remove "data: " prefix if present
		QString jsonLine = line;
		if (jsonLine.startsWith("data: ")) {
			jsonLine = jsonLine.mid(6);
		}
		
		// Check for stream end marker
		if (jsonLine.trimmed() == "[DONE]") continue;
		
		processStreamChunk(jsonLine);
	}
}

void NetworkManager::onFinished()
{
	if (!currentReply) return;

	// Process any remaining data in buffer
	if (!streamBuffer.isEmpty()) {
		QString remainingData = QString::fromUtf8(streamBuffer);
		if (remainingData.startsWith("data: ")) {
			remainingData = remainingData.mid(6);
		}
		if (!remainingData.trimmed().isEmpty() && remainingData.trimmed() != "[DONE]") {
			processStreamChunk(remainingData);
		}
	}

	QNetworkReply::NetworkError error = currentReply->error();
	
	// Clean up
	currentReply->deleteLater();
	currentReply = nullptr;
	streamBuffer.clear();

	if (error != QNetworkReply::NoError) {
		emit errorOccurred("Network request failed");
	}
	
	emit requestFinished();
}

void NetworkManager::onError(QNetworkReply::NetworkError error)
{
	Q_UNUSED(error)
	if (currentReply) {
		emit errorOccurred(currentReply->errorString());
	}
}

void NetworkManager::processStreamChunk(const QString &chunk)
{
	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(chunk.toUtf8(), &parseError);
	
	if (parseError.error != QJsonParseError::NoError) {
		return; // Skip invalid chunks
	}

	QJsonObject response = doc.object();
	
	// Check for errors
	if (response.contains("error")) {
		QJsonObject errorObj = response["error"].toObject();
		QString errorMessage = errorObj["message"].toString();
		emit errorOccurred("API Error: " + errorMessage);
		return;
	}

	// Extract delta content from streaming response
	QString deltaContent;
	if (response.contains("choices") && response["choices"].isArray()) {
		QJsonArray choices = response["choices"].toArray();
		if (!choices.isEmpty()) {
			QJsonObject choice = choices[0].toObject();
			if (choice.contains("delta")) {
				QJsonObject delta = choice["delta"].toObject();
				if (delta.contains("content")) {
					deltaContent = delta["content"].toString();
				}
			}
		}
	}

	if (!deltaContent.isEmpty()) {
		emit streamDataReceived(deltaContent);
	}
}