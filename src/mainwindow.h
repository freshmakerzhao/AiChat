#pragma once
#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QKeyEvent>

class ChatArea;

class MainWindow: public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow() override = default;

protected:
	bool eventFilter(QObject *obj,QEvent *ev) override;
	void resizeEvent(QResizeEvent *e) override;

private slots:
	void onSendClicked();

private:
	void addMessage(const QString &text,bool isUser);

private:
	QWidget *central = nullptr;
	// QListWidget *list = nullptr;   // old
	ChatArea *chatArea = nullptr;     // new
	QTextEdit *edit = nullptr;
	QPushButton *sendBtn = nullptr;
};