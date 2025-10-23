#include "BubbleDelegate.h"
#include <QListWidget>
#include <QStyle>

BubbleDelegate::BubbleDelegate(QObject *parent): QStyledItemDelegate(parent) {}

QSize BubbleDelegate::sizeHint(const QStyleOptionViewItem &opt,
			   const QModelIndex &idx) const
{
	if(auto lv = qobject_cast<const QListWidget*>(opt.widget)) {
		if(auto it = lv->item(idx.row())) {
			// If this item has a custom widget, directly return the widget's sizeHint
			if(QWidget *w = lv->itemWidget(it)) {
				const QSize ws = w->sizeHint();
				// Ensure minimum height to prevent layout issues
				return {opt.rect.width(), qMax(50, ws.height())};
			}
		}
	}

	// Only user messages need delegate to calculate size
	const bool isUser  = idx.data(IsUserRole).toBool();
	if (!isUser) {
		// AI messages should use AiBubbleWidget, should not reach here
		return {opt.rect.width(), 50}; // Increased minimum height
	}

	const QString text = idx.data(Qt::DisplayRole).toString();

	const QWidget *w = opt.widget;
	const int viewW  = w ? w->width() : opt.rect.width();
	if(viewW <= 0) return {viewW, 50};

	// User bubble: max 70% width, AI bubble: fixed 90% width
	const int userMaxBubbleW = qMax(kMinBubble,int(viewW * 0.70) - 2*kHMargin);

	// Use QTextDocument to accurately calculate height (consistent with rendering)
	QTextDocument doc;
	doc.setDefaultFont(opt.font);
	doc.setDocumentMargin(0);
	doc.setUndoRedoEnabled(false); // Performance optimization

	const int textWidth = userMaxBubbleW - 2*kPadding;
	doc.setTextWidth(textWidth);
	doc.setPlainText(text);
	
	// Calculate actual bubble width (shrink to content)
	const int idealWidth = int(std::ceil(doc.idealWidth())) + 2*kPadding;
	const int bubbleW = qMin(userMaxBubbleW, qMax(kMinBubble, idealWidth));
	const int bubbleH = int(std::ceil(doc.size().height())) + 2*kVPadding;

	// List item width: full row width; height: bubble height + spacing
	return {viewW, bubbleH + 12}; // Increased spacing
}

void BubbleDelegate::paint(QPainter *p,const QStyleOptionViewItem &opt,
		   const QModelIndex &idx) const
{
	if(!p || !p->isActive()) return;

	// If has custom widget, no need for delegate painting
	if(auto lv = qobject_cast<const QListWidget*>(opt.widget)) {
		if(auto it = lv->item(idx.row())) {
			if(lv->itemWidget(it)) {
				return; // Skip painting, let widget handle itself
			}
		}
	}

	const bool isUser  = idx.data(IsUserRole).toBool();
	if (!isUser) {
		return; // AI messages should use AiBubbleWidget
	}

	const QString text = idx.data(Qt::DisplayRole).toString();
	const int viewW = opt.rect.width();

	// Enhanced user bubble colors
	const QColor userBg = QColor(0x6a, 0x4c, 0xff);        // Blue
	const QColor userFg = Qt::white;
	const QColor userBorder = QColor(0x5a, 0x3c, 0xef);    // Darker blue border

	const int userMaxBubbleW = qMax(kMinBubble,int(viewW * 0.70) - 2*kHMargin);

	// Text layout: QTextDocument (sync with sizeHint)
	QTextDocument doc;
	doc.setDefaultFont(opt.font);
	doc.setDocumentMargin(0);
	doc.setUndoRedoEnabled(false);

	const int textWidth = userMaxBubbleW - 2*kPadding;
	doc.setTextWidth(textWidth);
	doc.setPlainText(text);
	
	const int idealWidth = int(std::ceil(doc.idealWidth())) + 2*kPadding;
	const int bubbleW = qMin(userMaxBubbleW, qMax(kMinBubble, idealWidth));
	const int bubbleH = int(std::ceil(doc.size().height())) + 2*kVPadding;

	// Position: user right-aligned, with top spacing
	QRect bubbleRect = QRect(opt.rect.right() - kHMargin - bubbleW,
						   opt.rect.top() + 6, // Increased top spacing
						   bubbleW, bubbleH);
	const QRect contentRect = bubbleRect.adjusted(kPadding,kVPadding,-kPadding,-kVPadding);

	// Draw base style (disable selection highlight/focus frame)
	QStyleOptionViewItem optBase(opt);
	optBase.text.clear();
	optBase.state &= ~QStyle::State_HasFocus;
	optBase.state &= ~QStyle::State_Selected;
	if(opt.widget && opt.widget->style()) {
		opt.widget->style()->drawControl(QStyle::CE_ItemViewItem,&optBase,p,opt.widget);
	}

	// Enhanced bubble drawing
	p->save();
	p->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);

	// Draw shadow for depth
	QRect shadowRect = bubbleRect.adjusted(2, 2, 2, 2);
	p->setPen(Qt::NoPen);
	p->setBrush(QColor(0, 0, 0, 20)); // Light shadow
	p->drawRoundedRect(shadowRect, kRadius, kRadius);

	// Draw bubble background with border
	p->setPen(QPen(userBorder, 1));
	p->setBrush(userBg);
	p->drawRoundedRect(bubbleRect, kRadius, kRadius);

	// Draw text using QTextDocument for consistency
	p->setPen(userFg);
	p->translate(contentRect.topLeft());
	QAbstractTextDocumentLayout::PaintContext ctx;
	ctx.palette.setColor(QPalette::Text, userFg);
	doc.documentLayout()->draw(p, ctx);
	p->restore();
}