#include "ChatBubble.h"
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QTimer>
#include <QTextDocument>
#include <QTextOption>
#include <cmath>

ChatBubble::ChatBubble(QWidget *parent)
	: QWidget(parent),
	m_browser(new QTextBrowser(this)),
	m_layout(new QVBoxLayout(this)),
	m_timer(new QTimer(this))
{
	setAttribute(Qt::WA_TranslucentBackground,true);
	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

	m_browser->setFrameShape(QFrame::NoFrame);
	m_browser->setOpenExternalLinks(true);
	m_browser->setReadOnly(true);
	m_browser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_browser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_browser->setStyleSheet("QTextBrowser { background: transparent; color: #212121; }");
	m_browser->document()->setDocumentMargin(0);

	QTextOption opt = m_browser->document()->defaultTextOption();
	opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
	m_browser->document()->setDefaultTextOption(opt);

	m_layout->setContentsMargins(12,6,12,6);
	m_layout->setSpacing(0);
	m_layout->addWidget(m_browser);
	setLayout(m_layout);

	m_timer->setSingleShot(true);
	m_timer->setInterval(60); // 节流间隔（ms），可按需调整
	connect(m_timer,&QTimer::timeout,this,&ChatBubble::applyPending);
	connect(m_browser->document(),&QTextDocument::contentsChanged,this,[this](){
		scheduleApply();
	});
}

ChatBubble::~ChatBubble() = default;

void ChatBubble::setMaxContentWidth(int w)
{
	if(w == m_maxContentWidth) return;
	m_maxContentWidth = w;
	scheduleApply();
}

void ChatBubble::appendStreamChunk(const QString &chunk)
{
	if(chunk.isEmpty()) return;
	m_raw += chunk;
	scheduleApply();
}

void ChatBubble::finalize()
{
	if(m_timer->isActive()) m_timer->stop();
	applyPending();
}

QSize ChatBubble::sizeHint() const
{
	int w = parentWidget() ? parentWidget()->width() : QWidget::sizeHint().width();
	int h = m_cachedHeight + m_layout->contentsMargins().top() + m_layout->contentsMargins().bottom();
	return QSize(w,qMax(40,h));
}

void ChatBubble::applyPending()
{
	// 将最新内容设置到 QTextBrowser（渲染 Markdown）
	m_browser->setMarkdown(m_raw);

	// 临时 QTextDocument 在指定宽度下测量高度
	QTextDocument tmp;
	tmp.setDefaultFont(m_browser->font());
	tmp.setDocumentMargin(0);
	QTextOption opt = tmp.defaultTextOption();
	opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
	tmp.setDefaultTextOption(opt);

	int contentW = m_maxContentWidth;
	if(contentW <= 0) {
		int avail = width() - (m_layout->contentsMargins().left() + m_layout->contentsMargins().right());
		contentW = qMax(80,avail);
	}

	tmp.setMarkdown(m_raw);
	tmp.setTextWidth(contentW);
	tmp.adjustSize();
	int contentH = int(std::ceil(tmp.size().height()));

	// 固定 browser 尺寸避免内部滚动
	m_browser->setFixedSize(contentW,contentH);

	m_cachedHeight = contentH;

	updateGeometry();
	update();

	emit needScrollToBottom();
}

void ChatBubble::scheduleApply()
{
	if(!m_timer->isActive()) m_timer->start();
}