#include "mainwindow.h"
#include "ChatArea.h"
#include "ChatBubble.h"
#include <QFontMetrics>
#include <QScrollBar>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowTitle(QStringLiteral("AI Chat - Minimal (Qt5.15 / C++17)"));
	resize(520,420);

	central = new QWidget(this);
	auto *mainLay = new QVBoxLayout(central);
	mainLay->setContentsMargins(8,8,8,8);
	mainLay->setSpacing(8);

	// ChatArea 替代原来的 QListWidget
	chatArea = new ChatArea(central);
	mainLay->addWidget(chatArea,1);

	// 输入区
	auto *inputRow = new QHBoxLayout;
	inputRow->setSpacing(8);
	edit = new QTextEdit(central);
	edit->setFixedHeight(70);
	edit->setPlaceholderText(QStringLiteral("input"));
	sendBtn = new QPushButton(QStringLiteral("send"),central);
	sendBtn->setDefault(true);
	inputRow->addWidget(edit,1);
	inputRow->addWidget(sendBtn);
	mainLay->addLayout(inputRow);

	setCentralWidget(central);

	// 事件
	connect(sendBtn,&QPushButton::clicked,this,&MainWindow::onSendClicked);
	edit->installEventFilter(this);
}

bool MainWindow::eventFilter(QObject *obj,QEvent *ev)
{
	if(obj == edit && ev->type() == QEvent::KeyPress) {
		auto *ke = static_cast<QKeyEvent*>(ev);
		if(ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
			if(ke->modifiers() & Qt::ShiftModifier) {
				return false; // Shift+Enter 换行
			}
			onSendClicked(); // Enter 发送
			return true;     // 吃掉回车
		}
	}
	return QMainWindow::eventFilter(obj,ev);
}

void MainWindow::onSendClicked()
{
	const QString text = edit->toPlainText().trimmed();
	if(text.isEmpty())
		return;

	edit->clear();

	// 1) 显示用户消息（右对齐），不需要流式
	ChatBubble *user = chatArea->addBubble(true);
	user->appendStreamChunk(text);
	user->finalize();

	// 2) 创建 AI 气泡并模拟流式（真实场景：把 LLM 的 chunk 信号 connect 到 ai->appendStreamChunk）
	ChatBubble *ai = chatArea->addBubble(false);

	// 这里用 QTimer 模拟分块流式到来；真实使用时从网络/LLM线程通过 queued signal 调用 appendStreamChunk
	const QString simulated = QStringLiteral("123");
	QTimer::singleShot(200,this,[ai,simulated]() {
		// 简化：按词分块
		const QStringList parts = simulated.split(' ');
		int t = 0;
		for(const QString &p : parts) {
			QTimer::singleShot(t,ai,[ai,p](){ ai->appendStreamChunk(p + " "); });
			t += 60;
		}
		// 流结束时 finalize
		QTimer::singleShot(t + 80,ai,[ai](){ ai->finalize(); });
	});
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
	QMainWindow::resizeEvent(e);
	// ChatArea 自身会在 resizeEvent 内下发宽度到每个 Bubble，因此这里不需要做额外事情。
}