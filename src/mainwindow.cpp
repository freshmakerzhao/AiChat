#include "mainwindow.h"
#include <QFontMetrics>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowTitle(QStringLiteral("AI Chat - Minimal (Qt5.15 / C++17)"));
	resize(520,420);

	central = new QWidget(this);
	auto *mainLay = new QVBoxLayout(central);
	mainLay->setContentsMargins(8,8,8,8);
	mainLay->setSpacing(8);

	// 消息列表
	list = new QListWidget(central);
	list->setWordWrap(true);
	list->setUniformItemSizes(false);
	list->setSelectionMode(QAbstractItemView::NoSelection);
	list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	list->setFrameShape(QFrame::NoFrame);
	list->setSpacing(6); // 行间距
	// 安装气泡委托（最关键）
	list->setItemDelegate(new BubbleDelegate(list));
	mainLay->addWidget(list,1);

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
	addMessage(text, /*isUser=*/true);

	QTimer::singleShot(300,this,[this,text](){
		addMessage(QStringLiteral("AI:over - ") + text, /*isUser=*/false);
	});
}

void MainWindow::addMessage(const QString &text,bool isUser)
{
	if(isUser) {
		auto *item = new QListWidgetItem(text);
		item->setData(BubbleDelegate::IsUserRole,true);
		list->addItem(item);
	} else {
		auto *item = new QListWidgetItem;
		list->addItem(item);                          // ① 先插入
		auto *w = new AiBubbleWidget(list);
		list->setItemWidget(item,w);                 // ② 再挂部件

		w->setHostItem(item);
		w->setViewportWidth(list->viewport()->width());
		w->setMarkdownText(text);                     // ③ 最后设内容（触发 recalc，现在已生效）
		// 可选：再来一次 layout，确保刷新
		list->doItemsLayout();
		list->scrollToBottom();
	}
	list->scrollToBottom();
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
	QMainWindow::resizeEvent(e);

	const int vw = list->viewport()->width();
	for(int i = 0; i < list->count(); ++i) {
		if(auto *w = list->itemWidget(list->item(i))) {
			if(auto *ai = qobject_cast<AiBubbleWidget*>(w)) {
				ai->setViewportWidth(vw);                       // ★ 会触发 recalc
				// 不必手动 setSizeHint，ai->recalc() 已经做了
			}
		}
	}
	list->doItemsLayout();
}