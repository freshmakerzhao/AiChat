#include "ChatManager.h"
#include <QJsonArray>

ChatManager::ChatManager(QObject *parent)
	: QObject(parent), maxTokens(2000), temperature(0.7), modelName("deepseek-chat")
{
}

void ChatManager::setApiConfiguration(const QString &key, const QString &url, const QString &model)
{
	apiKey = key;
	apiUrl = url;
	modelName = model;
}

void ChatManager::setModelParameters(int tokens, double temp)
{
	maxTokens = tokens;
	temperature = temp;
}

QJsonObject ChatManager::buildChatRequest(const QString &userMessage, const QString &systemPrompt) const
{
	QJsonObject json;
	json["model"] = modelName;
	json["stream"] = true;
	
	QJsonArray messages;
	
	// Add system message if provided
	QString prompt = systemPrompt.isEmpty() ? getDefaultSystemPrompt() : systemPrompt;
	if (!prompt.isEmpty()) {
		QJsonObject systemMessage;
		systemMessage["role"] = "system";
		systemMessage["content"] = prompt;
		messages.append(systemMessage);
	}
	
	// Add user message
	QJsonObject userMsg;
	userMsg["role"] = "user";
	userMsg["content"] = userMessage;
	messages.append(userMsg);
	
	json["messages"] = messages;
	json["max_tokens"] = maxTokens;
	json["temperature"] = temperature;
	
	return json;
}

QString ChatManager::formatErrorMessage(const QString &error)
{
	return QString("❌ **Error**\n\n%1").arg(error);
}

QString ChatManager::getDefaultSystemPrompt()
{
	return "You are a helpful AI assistant. Please answer all questions in Chinese. Support output in Markdown format.";
}