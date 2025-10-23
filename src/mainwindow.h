#pragma once
#include <QMainWindow>
#include "widgets/AiChatWidget.h"

class MainWindow: public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow() override = default;

private slots:
	void onMessageAdded(const QString &text, bool isUser);
	void onApiRequestStarted();
	void onApiRequestFinished();
	void onErrorOccurred(const QString &error);

private:
	AiChatWidget *chatWidget = nullptr;
};