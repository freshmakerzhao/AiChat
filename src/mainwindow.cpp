#include "mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowTitle(QStringLiteral("AI Chat"));
	resize(520, 420);

	// Create the chat widget
	chatWidget = new AiChatWidget(this);
	setCentralWidget(chatWidget);

	// Connect signals for monitoring
	connect(chatWidget, &AiChatWidget::messageAdded, this, &MainWindow::onMessageAdded);
	connect(chatWidget, &AiChatWidget::apiRequestStarted, this, &MainWindow::onApiRequestStarted);
	connect(chatWidget, &AiChatWidget::apiRequestFinished, this, &MainWindow::onApiRequestFinished);
	connect(chatWidget, &AiChatWidget::errorOccurred, this, &MainWindow::onErrorOccurred);
}

void MainWindow::onMessageAdded(const QString &text, bool isUser)
{
	qDebug() << "Message added:" << (isUser ? "User" : "AI") << text.left(50) + "...";
}

void MainWindow::onApiRequestStarted()
{
	setWindowTitle(QStringLiteral("AI Chat - Processing..."));
}

void MainWindow::onApiRequestFinished()
{
	setWindowTitle(QStringLiteral("AI Chat"));
}

void MainWindow::onErrorOccurred(const QString &error)
{
	qWarning() << "Chat error:" << error;
	setWindowTitle(QStringLiteral("AI Chat - Error"));
}