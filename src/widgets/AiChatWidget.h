#pragma once
#include <QWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QKeyEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDateTime>

class AiBubbleWidget;
class BubbleDelegate;

class AiChatWidget: public QWidget
{
	Q_OBJECT
public:
	explicit AiChatWidget(QWidget *parent = nullptr);
	~AiChatWidget() override = default;

	// Public interface for embedding
	void addMessage(const QString &text, bool isUser);
	void sendMessage(const QString &message);
	void clearChat();
	void setApiConfiguration(const QString &apiKey, const QString &apiUrl, const QString &model);

signals:
	void messageAdded(const QString &text, bool isUser);
	void apiRequestStarted();
	void apiRequestFinished();
	void errorOccurred(const QString &error);

protected:
	bool eventFilter(QObject *obj, QEvent *ev) override;
	void resizeEvent(QResizeEvent *e) override;

private slots:
	void onSendClicked();
	void onStreamTimer();
	void onStreamDataReceived();
	void onStreamFinished();

private:
	void setupUI();
	void setupConnections();
	void startStreamResponse(const QString &fullText);
	void sendApiRequest(const QString &userMessage);
	void updateAllItemHeights();
	void processStreamChunk(const QString &chunk);

private:
	// UI Components
	QListWidget *messageList = nullptr;
	QTextEdit *inputEdit = nullptr;
	QPushButton *sendButton = nullptr;
	
	// Network components
	QNetworkAccessManager *networkManager = nullptr;
	QNetworkReply *currentReply = nullptr;
	QByteArray streamBuffer;
	
	// Stream display
	QTimer *streamTimer = nullptr;
	QString currentStreamText;
	int streamPosition = 0;
	AiBubbleWidget *currentStreamWidget = nullptr;
	
	// Configuration
	QString apiKey;
	QString apiUrl;
	QString modelName;
	int maxTokens = 2000;
	double temperature = 0.7;
};