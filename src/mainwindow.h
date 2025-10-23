#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QKeyEvent>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QTextBrowser>
#include <QDebug>

class AiBubbleWidget: public QWidget {
	Q_OBJECT
public:
	explicit AiBubbleWidget(QWidget *parent=nullptr): QWidget(parent)
	{
		setAttribute(Qt::WA_TranslucentBackground,true);
		setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

		// 外层水平布局：留出左侧 hMargin 实现“左贴边”的效果
		outerLay = new QHBoxLayout(this);
		outerLay->setContentsMargins(kHMargin,0,kHMargin,0); // 左右边距、顶部微距
		outerLay->setSpacing(0);

		// 圆角气泡：使用 QFrame + 样式表，不再自绘
		bubble = new QFrame(this);
		bubble->setObjectName("AiBubble");
		bubble->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
		bubble->setStyleSheet(
			"QFrame#AiBubble {"
			"  background: rgb(245,245,245);"
			"  border-radius: 12px;"
			"}"
		);
		outerLay->addWidget(bubble,0,Qt::AlignLeft); // 靠左

		// 气泡内布局：实现 padding
		innerLay = new QVBoxLayout(bubble);
		innerLay->setContentsMargins(kPadding,kVPadding,kPadding,kVPadding);
		innerLay->setSpacing(0);

		// Markdown 展示器
		browser = new QTextBrowser(bubble);
		browser->setFrameShape(QFrame::NoFrame);
		browser->setOpenExternalLinks(true);
		browser->setReadOnly(true);
		browser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		browser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		browser->setLineWrapMode(QTextEdit::WidgetWidth);
		browser->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
		browser->setStyleSheet("QTextBrowser { background: transparent; color: #212121; }");
		browser->document()->setDocumentMargin(0);

		QTextOption opt = browser->document()->defaultTextOption();
		opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
		browser->document()->setDefaultTextOption(opt);

		// 文档变化重新测量
		connect(browser->document(),&QTextDocument::contentsChanged,this,[this](){
			recalc();
		});

		innerLay->addWidget(browser);
	}

	void setMarkdownText(const QString &md) {
		rawMd = md;
		browser->setMarkdown(md);
		recalc();
	}

	void setViewportWidth(int vw) {
		viewW = vw;
		recalc();
	}

	void setHostItem(QListWidgetItem *it) {
		hostItem = it;
	}

	QSize sizeHint() const override {
		const int bubbleW  = calcBubbleWidth();
		const int contentW = bubbleW - 2*kPadding;

		// 用独立 QTextDocument 计算高度，避免被 QTextBrowser 干扰
		QTextDocument tmp;
		tmp.setDefaultFont(browser->font());
		tmp.setDocumentMargin(0);
		QTextOption opt = tmp.defaultTextOption();
		opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
		tmp.setDefaultTextOption(opt);
		tmp.setMarkdown(rawMd);
		tmp.setTextWidth(contentW);
		tmp.adjustSize();
		const int contentH = int(std::ceil(tmp.size().height()));

		const int bubbleH  = contentH + 2*kVPadding;
		const int totalH   = bubbleH + 3 + 6;
		return QSize(bubbleW + 2*kHMargin,totalH);
	}

protected:
	void resizeEvent(QResizeEvent *e) override {
		QWidget::resizeEvent(e);
		// 这里无需自绘，布局会根据我们在 recalc 中设置的 fixedWidth/height 自动生效
	}

private:
	// 常量
	QString rawMd;  // 保存 setMarkdownText 传入的原始内容
	static constexpr int kHMargin   = 10;  // 外留白
	static constexpr int kPadding   = 10;  // 气泡内左右留白
	static constexpr int kVPadding  = 8;   // 气泡内上下留白
	static constexpr int kMinBubble = 120; // 最小气泡宽度

	int calcBubbleWidth() const {
		const int vw = (viewW > 0 ? viewW : width());
		if(vw <= 0) return kMinBubble;
		// 固定 95%（再扣掉我们 outerLay 的左右边距 kHMargin）
		return qMax(kMinBubble,int(vw * 0.95) - 2*kHMargin);
	}

	void ensureMeasured() const {
		if(measured) return;
		const_cast<AiBubbleWidget*>(this)->recalc();
	}

	void recalc() {
		const int bubbleW  = calcBubbleWidth();
		const int contentW = bubbleW - 2*kPadding;

		// —— 独立文档做测量 —— //
		QTextDocument tmp;
		tmp.setDefaultFont(browser->font());
		tmp.setDocumentMargin(0);
		QTextOption opt = tmp.defaultTextOption();
		opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
		tmp.setDefaultTextOption(opt);
		tmp.setMarkdown(rawMd);
		tmp.setTextWidth(contentW);
		tmp.adjustSize();
		const int contentH = int(std::ceil(tmp.size().height()));

		// 1) 禁滚动：控件尺寸 = 内容尺寸
		browser->setFixedSize(contentW,contentH);

		// 2) 固定气泡宽度
		bubble->setFixedWidth(bubbleW);

		// 3) 更新 item 的 sizeHint
		const int bubbleH = contentH + 2*kVPadding;
		const int totalH  = bubbleH;

		cachedSizeHint = QSize(bubbleW + 2*kHMargin,totalH);
		measured = true;

		if(hostItem) hostItem->setSizeHint(cachedSizeHint);
		if(auto *lw = qobject_cast<QListWidget*>(parentWidget())) {
			lw->doItemsLayout();
			lw->scrollToBottom();
		}
		updateGeometry();
		update();
		qDebug().noquote()
			<< "[AI] vw=" << viewW
			<< " bubbleW=" << bubbleW
			<< " contentW=" << contentW
			<< " contentH=" << contentH;
	}

private:
	// 结构
	QHBoxLayout *outerLay = nullptr;
	QVBoxLayout *innerLay = nullptr;
	QFrame      *bubble   = nullptr;
	QTextBrowser *browser = nullptr;

	int viewW = 0;
	QListWidgetItem *hostItem = nullptr;

	// 缓存
	QSize cachedSizeHint{200,40};
	bool  measured = false;
};



class BubbleDelegate: public QStyledItemDelegate
{
public:
	enum {
		IsUserRole = Qt::UserRole + 1
	};
	explicit BubbleDelegate(QObject *parent=nullptr): QStyledItemDelegate(parent) {}

	static constexpr int kHMargin   = 10;
	static constexpr int kPadding   = 10;
	static constexpr int kVPadding  = 8;
	static constexpr int kRadius    = 12;
	static constexpr int kMinBubble = 120;

	QSize sizeHint(const QStyleOptionViewItem &opt,
				   const QModelIndex &idx) const override
	{
		if(auto lv = qobject_cast<const QListWidget*>(opt.widget)) {        // const 版本
			if(auto it = lv->item(idx.row())) {                              // const QListWidgetItem*
				if(QWidget *w = lv->itemWidget(it)) {                        // OK：这个函数是 const 成员
					const QSize ws = w->sizeHint();
					return {opt.rect.width(),ws.height()};
				}
			}
		}

		const bool isUser  = idx.data(IsUserRole).toBool();
		const QString text = idx.data(Qt::DisplayRole).toString();

		const QWidget *w = opt.widget;
		const int viewW  = w ? w->width() : opt.rect.width();
		if(viewW <= 0) return {0,0};

		const int userMaxBubbleW = qMax(kMinBubble,int(viewW * 0.70) - 2*kHMargin);
		const int aiFixedBubbleW = qMax(kMinBubble,int(viewW * 0.95) - 2*kHMargin);

		// 用 QTextDocument 精确计算高度（与绘制一致）
		QTextDocument doc;
		doc.setDefaultFont(opt.font);
		doc.setDocumentMargin(0);

		int bubbleW = 0;
		if(isUser) {
			const int textWidth = userMaxBubbleW - 2*kPadding;
			doc.setTextWidth(textWidth);
			doc.setPlainText(text);
			// 用户气泡按内容收缩，但不超过 70%
			bubbleW = qMin(userMaxBubbleW,int(std::ceil(doc.idealWidth())) + 2*kPadding);
		} else {
			const int textWidth = aiFixedBubbleW - 2*kPadding;
			doc.setTextWidth(textWidth);
			doc.setPlainText(text);
			bubbleW = aiFixedBubbleW; // 固定 95%
		}

		const int bubbleH = int(std::ceil(doc.size().height())) + 2*kVPadding;

		// 列表项宽：整行宽；高：气泡高 + 行间距
		return {viewW,bubbleH };
	}

	void paint(QPainter *p,const QStyleOptionViewItem &opt,
			   const QModelIndex &idx) const override
	{
		if(!p || !p->isActive()) return;

		const bool isUser  = idx.data(IsUserRole).toBool();
		const QString text = idx.data(Qt::DisplayRole).toString();

		const int viewW = opt.rect.width();

		const QColor userBg = QColor(QStringLiteral("#6a4cff"));
		const QColor userFg = Qt::white;
		const QColor aiBg   = QColor(245,245,245);
		const QColor aiFg   = QColor(33,33,33);

		const int userMaxBubbleW = qMax(kMinBubble,int(viewW * 0.70) - 2*kHMargin);
		const int aiFixedBubbleW = qMax(kMinBubble,int(viewW * 0.95) - 2*kHMargin);

		// 文本布局：QTextDocument（与 sizeHint 同步）
		QTextDocument doc;
		doc.setDefaultFont(opt.font);
		doc.setDocumentMargin(0);

		int bubbleW = 0;
		if(isUser) {
			const int textWidth = userMaxBubbleW - 2*kPadding;
			doc.setTextWidth(textWidth);
			doc.setPlainText(text);
			bubbleW = qMin(userMaxBubbleW,int(std::ceil(doc.idealWidth())) + 2*kPadding);
		} else {
			const int textWidth = aiFixedBubbleW - 2*kPadding;
			doc.setTextWidth(textWidth);
			doc.setPlainText(text);
			bubbleW = aiFixedBubbleW;
		}

		const int bubbleH = int(std::ceil(doc.size().height())) + 2*kVPadding;

		// 位置：用户右、AI 左
		QRect bubbleRect;
		if(isUser) {
			bubbleRect = QRect(opt.rect.right() - kHMargin - bubbleW,
							   opt.rect.top() + 3,
							   bubbleW,bubbleH);
		} else {
			bubbleRect = QRect(opt.rect.left() + kHMargin,
							   opt.rect.top() + 3,
							   bubbleW,bubbleH);
		}
		const QRect contentRect = bubbleRect.adjusted(kPadding,kVPadding,-kPadding,-kVPadding);

		// 先让样式绘制基础（禁用选中高亮/焦点框）
		QStyleOptionViewItem optBase(opt);
		optBase.text.clear();
		optBase.state &= ~QStyle::State_HasFocus;
		optBase.state &= ~QStyle::State_Selected;
		if(opt.widget && opt.widget->style()) {
			opt.widget->style()->drawControl(QStyle::CE_ItemViewItem,&optBase,p,opt.widget);
		}

		// 画气泡 + 文本
		p->save();
		p->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing,true);

		p->setPen(Qt::NoPen);
		p->setBrush(isUser ? userBg : aiBg);
		p->drawRoundedRect(bubbleRect,kRadius,kRadius);

		// 用 QTextDocument 绘制，确保与测量一致
		p->setPen(isUser ? userFg : aiFg);
		p->translate(contentRect.topLeft());
		QAbstractTextDocumentLayout::PaintContext ctx;
		// 设置前景色（也可以给 doc 的 defaultTextOption 设置，但这里简单）
		ctx.palette.setColor(QPalette::Text,isUser ? userFg : aiFg);
		doc.documentLayout()->draw(p,ctx);
		p->restore();
	}
};


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
	void updateAllItemHeights(); // 窗口尺寸变化时重算气泡高度

private:
	QWidget *central = nullptr;
	QListWidget *list = nullptr;
	QTextEdit *edit = nullptr;
	QPushButton *sendBtn = nullptr;
};
