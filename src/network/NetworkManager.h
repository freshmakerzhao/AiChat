#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QJsonObject>

class NetworkManager : public QObject
{
	Q_OBJECT
public:
	explicit NetworkManager(QObject *parent = nullptr);
	
	void sendStreamingRequest(const QString &apiKey, const QString &apiUrl, 
							 const QJsonObject &requestData);
	void cancelCurrentRequest();
	
	bool isBusy() const { return currentReply != nullptr; }

signals:
	void streamDataReceived(const QString &deltaContent);
	void requestFinished();
	void errorOccurred(const QString &error);

private slots:
	void onReadyRead();
	void onFinished();
	void onError(QNetworkReply::NetworkError error);

private:
	void processStreamChunk(const QString &chunk);

private:
	QNetworkAccessManager *manager;
	QNetworkReply *currentReply;
	QByteArray streamBuffer;
};