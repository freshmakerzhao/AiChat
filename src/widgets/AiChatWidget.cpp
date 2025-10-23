#include "AiChatWidget.h"
#include "AiBubbleWidget.h"
#include "BubbleDelegate.h"
#include <utils/Config.h>
#include <QFontMetrics>
#include <QScrollBar>
#include <QTimer>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QDebug>

AiChatWidget::AiChatWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUI();
	setupConnections();
	
	// Load default configuration
	apiKey = Config::getDeepSeekApiKey();
	apiUrl = Config::getDeepSeekApiUrl();
	modelName = Config::getModelName();
	maxTokens = Config::getMaxTokens();
	temperature = Config::getTemperature();
}

void AiChatWidget::setupUI()
{
	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0); // No margins for embedding
	mainLayout->setSpacing(8);

	// Message list
	messageList = new QListWidget(this);
	messageList->setWordWrap(true);
	messageList->setUniformItemSizes(false);
	messageList->setSelectionMode(QAbstractItemView::NoSelection);
	messageList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	messageList->setFrameShape(QFrame::NoFrame);
	messageList->setSpacing(4);
	
	// Enable smooth scrolling
	messageList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	messageList->verticalScrollBar()->setSingleStep(3);
	
	// Styling for better appearance
	messageList->setFocusPolicy(Qt::NoFocus);
	messageList->setStyleSheet(
		"QListWidget {"
		"  background-color: transparent;"
		"  border: none;"
		"  outline: none;"
		"}"
		"QListWidget::item {"
		"  border: none;"
		"  background: transparent;"
		"}"
		"QListWidget::item:selected {"
		"  background: transparent;"
		"}"
	);
	
	// Install bubble delegate
	messageList->setItemDelegate(new BubbleDelegate(messageList));
	mainLayout->addWidget(messageList, 1);

	// Input area
	auto *inputLayout = new QHBoxLayout;
	inputLayout->setSpacing(8);
	
	inputEdit = new QTextEdit(this);
	inputEdit->setFixedHeight(70);
	inputEdit->setPlaceholderText(QStringLiteral("Type your message here..."));
	inputEdit->setStyleSheet(
		"QTextEdit {"
		"  background-color: transparent;"
		"  border: 2px solid #e0e0e0;"
		"  border-radius: 12px;"
		"  padding: 8px;"
		"  font-size: 14px;"
		"}"
		"QTextEdit:focus {"
		"  border-color: #5b66f1;"
		"  background-color: #ffffff;"
		"}"
	);
	
	sendButton = new QPushButton(QStringLiteral("Send"), this);
	sendButton->setDefault(true);
	sendButton->setFixedSize(80, 70);
	sendButton->setStyleSheet(
		"QPushButton {"
		"  background-color: #5b66f1;"
		"  color: white;"
		"  border: none;"
		"  border-radius: 12px;"
		"  font-size: 14px;"
		"  font-weight: bold;"
		"}"
		"QPushButton:hover {"
		"  background-color: #4b56e1;"
		"}"
		"QPushButton:pressed {"
		"  background-color: #3b46d1;"
		"}"
		"QPushButton:disabled {"
		"  background-color: #cccccc;"
		"}"
	);
	
	inputLayout->addWidget(inputEdit, 1);
	inputLayout->addWidget(sendButton);
	mainLayout->addLayout(inputLayout);
}

void AiChatWidget::setupConnections()
{
	// Initialize network manager
	networkManager = new QNetworkAccessManager(this);

	// Initialize stream timer
	streamTimer = new QTimer(this);
	streamTimer->setSingleShot(false);
	connect(streamTimer, &QTimer::timeout, this, &AiChatWidget::onStreamTimer);

	// UI connections
	connect(sendButton, &QPushButton::clicked, this, &AiChatWidget::onSendClicked);
	inputEdit->installEventFilter(this);
}

bool AiChatWidget::eventFilter(QObject *obj, QEvent *ev)
{
	if (obj == inputEdit && ev->type() == QEvent::KeyPress) {
		auto *ke = static_cast<QKeyEvent*>(ev);
		if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
			if (ke->modifiers() & Qt::ShiftModifier) {
				return false; // Shift+Enter for line break
			}
			onSendClicked(); // Enter to send
			return true;     // Consume the enter key
		}
	}
	return QWidget::eventFilter(obj, ev);
}

void AiChatWidget::onSendClicked()
{
	const QString text = inputEdit->toPlainText().trimmed();
	if (text.isEmpty()) return;

	inputEdit->clear();
	sendMessage(text);
}

void AiChatWidget::addMessage(const QString &text, bool isUser)
{
	if (isUser) {
		auto *item = new QListWidgetItem(text);
		item->setData(BubbleDelegate::IsUserRole, true);
		messageList->addItem(item);
	} else {
		auto *item = new QListWidgetItem;
		messageList->addItem(item);
		auto *widget = new AiBubbleWidget(messageList);
		messageList->setItemWidget(item, widget);

		widget->setHostItem(item);
		widget->setViewportWidth(messageList->viewport()->width());
		widget->setMarkdownText(text);
		messageList->doItemsLayout();
		messageList->scrollToBottom();
	}
	messageList->scrollToBottom();
	
	emit messageAdded(text, isUser);
}

void AiChatWidget::sendMessage(const QString &message)
{
	addMessage(message, true);
	sendApiRequest(message);
}

void AiChatWidget::clearChat()
{
	messageList->clear();
}

void AiChatWidget::setApiConfiguration(const QString &key, const QString &url, const QString &model)
{
	apiKey = key;
	apiUrl = url;
	modelName = model;
}

void AiChatWidget::resizeEvent(QResizeEvent *e)
{
	QWidget::resizeEvent(e);

	// Simple batched update to avoid layout thrashing
	static QTimer* resizeTimer = nullptr;
	if (!resizeTimer) {
		resizeTimer = new QTimer(this);
		resizeTimer->setSingleShot(true);
		resizeTimer->setInterval(50);
		connect(resizeTimer, &QTimer::timeout, [this]() {
			const int vw = messageList->viewport()->width();
			for (int i = 0; i < messageList->count(); ++i) {
				if (auto *widget = messageList->itemWidget(messageList->item(i))) {
					if (auto *ai = qobject_cast<AiBubbleWidget*>(widget)) {
						ai->setViewportWidth(vw);
					}
				}
			}
			messageList->doItemsLayout();
		});
	}
	
	if (!resizeTimer->isActive()) {
		resizeTimer->start();
	}
}

void AiChatWidget::sendApiRequest(const QString &userMessage)
{
	// Cancel any ongoing request
	if (currentReply) {
		currentReply->abort();
		currentReply->deleteLater();
		currentReply = nullptr;
	}

	if (apiKey.isEmpty() || apiKey == "sk-your-deepseek-api-key-here") {
		startStreamResponse("**Configuration Error**\n\nPlease configure DeepSeek API Key first.");
		emit errorOccurred("API Key not configured");
		return;
	}

	// Disable send button during request
	sendButton->setEnabled(false);
	emit apiRequestStarted();

	// Build request JSON
	QJsonObject json;
	json["model"] = modelName;
	json["stream"] = true;
	
	QJsonArray messages;
	QJsonObject systemMessage;
	systemMessage["role"] = "system";
	systemMessage["content"] = "You are a helpful AI assistant. Please answer all questions in Chinese. Support output in Markdown format.";
	messages.append(systemMessage);
	
	QJsonObject userMsg;
	userMsg["role"] = "user";
	userMsg["content"] = userMessage;
	messages.append(userMsg);
	
	json["messages"] = messages;
	json["max_tokens"] = maxTokens;
	json["temperature"] = temperature;

	// Create request
	QNetworkRequest request;
	request.setUrl(QUrl(apiUrl));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());
	request.setRawHeader("User-Agent", "Qt-AI-Chat/1.0");

	// Send request
	QJsonDocument doc(json);
	currentReply = networkManager->post(request, doc.toJson());
	
	// Initialize streaming response
	currentStreamText.clear();
	streamPosition = 0;
	
	// Create AI bubble for streaming
	auto *item = new QListWidgetItem;
	messageList->addItem(item);
	currentStreamWidget = new AiBubbleWidget(messageList);
	messageList->setItemWidget(item, currentStreamWidget);
	currentStreamWidget->setHostItem(item);
	currentStreamWidget->setViewportWidth(messageList->viewport()->width());
	currentStreamWidget->setMarkdownText("");
	messageList->doItemsLayout();
	messageList->scrollToBottom();
	
	// Connect signals for streaming response
	connect(currentReply, &QNetworkReply::readyRead, this, &AiChatWidget::onStreamDataReceived);
	connect(currentReply, &QNetworkReply::finished, this, &AiChatWidget::onStreamFinished);
	connect(currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
		[this](QNetworkReply::NetworkError error) {
			qWarning() << "Network error:" << error << currentReply->errorString();
			emit errorOccurred(QString("Network error: %1").arg(currentReply->errorString()));
		});
}

void AiChatWidget::startStreamResponse(const QString &fullText)
{
	// Create AI bubble and set text directly (for error messages)
	auto *item = new QListWidgetItem;
	messageList->addItem(item);
	currentStreamWidget = new AiBubbleWidget(messageList);
	messageList->setItemWidget(item, currentStreamWidget);
	currentStreamWidget->setHostItem(item);
	currentStreamWidget->setViewportWidth(messageList->viewport()->width());
	currentStreamWidget->setMarkdownText(fullText);
	messageList->doItemsLayout();
	messageList->scrollToBottom();
}

void AiChatWidget::onStreamTimer()
{
	// This method is kept for potential future use
}

void AiChatWidget::onStreamDataReceived()
{
	if (!currentReply || !currentStreamWidget) {
		return;
	}

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
		if (line.trimmed().isEmpty()) {
			continue;
		}
		
		// Remove "data: " prefix if present
		QString jsonLine = line;
		if (jsonLine.startsWith("data: ")) {
			jsonLine = jsonLine.mid(6);
		}
		
		// Check for stream end marker
		if (jsonLine.trimmed() == "[DONE]") {
			continue;
		}
		
		processStreamChunk(jsonLine);
	}
}

void AiChatWidget::onStreamFinished()
{
	if (!currentReply) {
		return;
	}

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

	// Re-enable send button
	sendButton->setEnabled(true);
	emit apiRequestFinished();

	if (error != QNetworkReply::NoError) {
		if (currentStreamWidget) {
			currentStreamWidget->setMarkdownText("❌ **Network Error**\n\nRequest failed. Please try again.");
		}
		emit errorOccurred("Network request failed");
		return;
	}

	// Finalize the stream display
	if (currentStreamWidget) {
		messageList->doItemsLayout();
		messageList->scrollToBottom();
	}
	
	currentStreamWidget = nullptr;
}

void AiChatWidget::processStreamChunk(const QString &chunk)
{
	if (!currentStreamWidget) {
		return;
	}

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
		currentStreamWidget->setMarkdownText("❌ **API Error**\n\n" + errorMessage);
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
		// Append new content to current stream text
		currentStreamText.append(deltaContent);
		
		// Use optimized streaming update
		currentStreamWidget->updateMarkdownTextStreaming(currentStreamText);
		
		// Simple rate limiting for auto-scroll to improve performance
		static qint64 lastScrollTime = 0;
		qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
		if (currentTime - lastScrollTime > 100) { // Max 10 scrolls per second
			messageList->scrollToBottom();
			lastScrollTime = currentTime;
		}
	}
}

void AiChatWidget::updateAllItemHeights()
{
	// This method can be called externally to refresh all bubbles
	const int vw = messageList->viewport()->width();
	for (int i = 0; i < messageList->count(); ++i) {
		if (auto *widget = messageList->itemWidget(messageList->item(i))) {
			if (auto *ai = qobject_cast<AiBubbleWidget*>(widget)) {
				ai->setViewportWidth(vw);
			}
		}
	}
	messageList->doItemsLayout();
}