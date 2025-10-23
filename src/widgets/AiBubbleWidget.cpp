#include "AiBubbleWidget.h"
#include <QResizeEvent>
#include <QMetaObject>
#include <cmath>

AiBubbleWidget::AiBubbleWidget(QWidget *parent): QWidget(parent)
{
	setAttribute(Qt::WA_TranslucentBackground,true);
	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

	// External horizontal layout: leave left hMargin to achieve "left-aligned" effect
	outerLay = new QHBoxLayout(this);
	outerLay->setContentsMargins(kHMargin,8,kHMargin,12); // Increased bottom margin
	outerLay->setSpacing(0);

	// Rounded bubble: use QFrame + stylesheet for proper chat bubble appearance
	bubble = new QFrame(this);
	bubble->setObjectName("AiBubble");
	bubble->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed); // Fixed size to prevent layout issues
	bubble->setAttribute(Qt::WA_TranslucentBackground,false);
	bubble->setAttribute(Qt::WA_StyledBackground,true);
	bubble->setStyleSheet(
		"QFrame#AiBubble {"
		"  background-color: #f0f0f0;"
		"  border: 1px solid #e0e0e0;"
		"  border-radius: 12px;"
		"  margin: 0px;"
		"  padding: 0px;"
		"}"
	);
	outerLay->addWidget(bubble,0,Qt::AlignLeft); // Align left

	// Bubble internal layout: implement padding
	innerLay = new QVBoxLayout(bubble);
	innerLay->setContentsMargins(kPadding,kVPadding,kPadding,kVPadding);
	innerLay->setSpacing(0);

	// Markdown display with disabled interactions
	browser = new QTextBrowser(bubble);
	browser->setFrameShape(QFrame::NoFrame);
	browser->setOpenExternalLinks(true);
	browser->setReadOnly(true);
	browser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	browser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	browser->setLineWrapMode(QTextEdit::WidgetWidth);
	browser->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	browser->setStyleSheet("QTextBrowser { background: transparent; color: #212121; border: none; }");
	browser->document()->setDocumentMargin(0);
	
	// Disable all mouse interactions to prevent scrolling
	browser->setContextMenuPolicy(Qt::NoContextMenu);
	browser->setFocusPolicy(Qt::NoFocus);
	browser->setTextInteractionFlags(Qt::NoTextInteraction);
	browser->viewport()->setAttribute(Qt::WA_TransparentForMouseEvents);

	QTextOption opt = browser->document()->defaultTextOption();
	opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
	browser->document()->setDefaultTextOption(opt);

	// Use timer for delayed recalculation to prevent layout thrashing
	recalcTimer = new QTimer(this);
	recalcTimer->setSingleShot(true);
	recalcTimer->setInterval(10); // 10ms delay
	connect(recalcTimer, &QTimer::timeout, this, &AiBubbleWidget::performRecalc);

	// Document change triggers delayed re-measurement
	connect(browser->document(),&QTextDocument::contentsChanged,this,[this](){
		if (!recalcTimer->isActive()) {
			recalcTimer->start();
		}
	});

	innerLay->addWidget(browser);
}

void AiBubbleWidget::setMarkdownText(const QString &md) {
	rawMd = md;
	browser->setMarkdown(md);
	// Use timer to prevent immediate layout thrashing
	if (!recalcTimer->isActive()) {
		recalcTimer->start();
	}
}

void AiBubbleWidget::updateMarkdownTextStreaming(const QString &md) {
	// Simple optimization for streaming: reduce unnecessary recalculations
	if (md == rawMd) return; // No change, skip update
	
	rawMd = md;
	browser->setMarkdown(md);
	
	// Only recalculate if text length changes significantly (reduces layout updates)
	static int lastLength = 0;
	int currentLength = md.length();
	if (qAbs(currentLength - lastLength) > 20 || currentLength < lastLength) {
		if (!recalcTimer->isActive()) {
			recalcTimer->start();
		}
		lastLength = currentLength;
	}
}

void AiBubbleWidget::setViewportWidth(int vw) {
	if (viewW != vw) {
		viewW = vw;
		// Use timer to prevent immediate layout updates
		if (!recalcTimer->isActive()) {
			recalcTimer->start();
		}
	}
}

void AiBubbleWidget::setHostItem(QListWidgetItem *it) {
	hostItem = it;
}

QSize AiBubbleWidget::sizeHint() const {
	if (!measured) {
		// Calculate size on demand
		const_cast<AiBubbleWidget*>(this)->calculateSizeHint();
	}
	return cachedSizeHint;
}

void AiBubbleWidget::performRecalc() {
	calculateSizeHint();
	
	const int bubbleW  = calcBubbleWidth();
	const int contentW = bubbleW - 2*kPadding;
	const int contentH = cachedContentHeight;

	// Apply fixed sizes to prevent widget expansion
	browser->setFixedSize(contentW, contentH);
	bubble->setFixedSize(bubbleW, contentH + 2*kVPadding);

	// Update item size hint
	if(hostItem) {
		hostItem->setSizeHint(cachedSizeHint);
	}
	
	// Single layout update at the end
	if(auto *lw = qobject_cast<QListWidget*>(parentWidget())) {
		// Use queued connection to prevent recursive layout calls
		QMetaObject::invokeMethod(lw, "doItemsLayout", Qt::QueuedConnection);
	}
	
	updateGeometry();
}

void AiBubbleWidget::resizeEvent(QResizeEvent *e) {
	QWidget::resizeEvent(e);
	// Trigger recalculation when widget is resized
	if (!recalcTimer->isActive()) {
		recalcTimer->start();
	}
}

void AiBubbleWidget::calculateSizeHint() {
	const int bubbleW  = calcBubbleWidth();
	const int contentW = bubbleW - 2*kPadding;

	// Use independent QTextDocument to calculate height with caching
	static QTextDocument tmp;
	tmp.setDefaultFont(browser->font());
	tmp.setDocumentMargin(0);
	QTextOption opt = tmp.defaultTextOption();
	opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
	tmp.setDefaultTextOption(opt);
	tmp.setMarkdown(rawMd);
	tmp.setTextWidth(contentW);
	tmp.adjustSize();
	
	cachedContentHeight = int(std::ceil(tmp.size().height()));
	const int bubbleH = cachedContentHeight + 2*kVPadding;
	const int totalH = bubbleH + 20; // Increased margin to prevent overlap
	
	cachedSizeHint = QSize(bubbleW + 2*kHMargin, totalH);
	measured = true;
}

int AiBubbleWidget::calcBubbleWidth() const {
	const int vw = (viewW > 0 ? viewW : width());
	if(vw <= 0) return kMinBubble;
	// Fixed 90% for better appearance
	return qMax(kMinBubble,int(vw * 0.90) - 2*kHMargin);
}

void AiBubbleWidget::ensureMeasured() const {
	if(measured) return;
	const_cast<AiBubbleWidget*>(this)->calculateSizeHint();
}