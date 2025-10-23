#pragma once
#include <QObject>
#include <QString>
#include <QJsonObject>

class ChatManager : public QObject
{
	Q_OBJECT
public:
	explicit ChatManager(QObject *parent = nullptr);
	
	// Configuration
	void setApiConfiguration(const QString &apiKey, const QString &apiUrl, 
							const QString &model = "deepseek-chat");
	void setModelParameters(int maxTokens = 2000, double temperature = 0.7);
	
	// Request building
	QJsonObject buildChatRequest(const QString &userMessage, 
								const QString &systemPrompt = QString()) const;
	
	// Utility methods
	static QString formatErrorMessage(const QString &error);
	static QString getDefaultSystemPrompt();

private:
	QString apiKey;
	QString apiUrl;
	QString modelName;
	int maxTokens;
	double temperature;
};