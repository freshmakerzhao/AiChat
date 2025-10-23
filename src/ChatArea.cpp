#include "ChatArea.h"
#include "ChatBubble.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QTimer>
#include <QMargins>

ChatArea::ChatArea(QWidget *parent)
	: QScrollArea(parent),
	m_container(new QWidget),
	m_layout(new QVBoxLayout)
{
	setWidgetResizable(true);
	setFrameShape(QFrame::NoFrame);

	m_layout->setAlignment(Qt::AlignTop);
	m_layout->setSpacing(8);
	m_layout->setContentsMargins(8,8,8,8);

	m_container->setLayout(m_layout);
	setWidget(m_container);
}

ChatBubble* ChatArea::addBubble(bool isUser)
{
	ChatBubble *b = new ChatBubble(m_container);
	connect(b,&ChatBubble::needScrollToBottom,this,&ChatArea::onBubbleNeedScroll,Qt::QueuedConnection);

	QWidget *row = new QWidget(m_container);
	QHBoxLayout *rowLay = new QHBoxLayout(row);
	rowLay->setContentsMargins(0,0,0,0);
	rowLay->setSpacing(0);

	if(isUser) {
		rowLay->addStretch();
		rowLay->addWidget(b,0,Qt::AlignRight);
	} else {
		rowLay->addWidget(b,0,Qt::AlignLeft);
		rowLay->addStretch();
	}

	row->setLayout(rowLay);
	m_layout->addWidget(row);

	// 下发初始可用 content 宽度
	b->setMaxContentWidth(calcContentWidth());

	// 确保滚到底（延后到事件循环末）
	QTimer::singleShot(0,this,[this](){ verticalScrollBar()->setValue(verticalScrollBar()->maximum()); });

	return b;
}

void ChatArea::resizeEvent(QResizeEvent *ev)
{
	QScrollArea::resizeEvent(ev);

	const int cw = calcContentWidth();
	for(ChatBubble *b : m_container->findChildren<ChatBubble*>()) {
		if(b) b->setMaxContentWidth(cw);
	}

	if(m_container->layout()) {
		m_container->layout()->invalidate();
		m_container->layout()->activate();
	}
}

void ChatArea::onBubbleNeedScroll()
{
	verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

int ChatArea::calcContentWidth() const
{
	int vw = viewport()->width();
	QMargins m = m_layout->contentsMargins();
	int avail = vw - (m.left() + m.right());
	const int bubblePaddingLR = 12 * 2;
	int contentW = qMax(80,avail - bubblePaddingLR);
	return contentW;
}