#pragma once
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QListWidget>
#include <cmath>

class BubbleDelegate: public QStyledItemDelegate
{
public:
	enum {
		IsUserRole = Qt::UserRole + 1
	};
	explicit BubbleDelegate(QObject *parent=nullptr);

	QSize sizeHint(const QStyleOptionViewItem &opt,
				   const QModelIndex &idx) const override;

	void paint(QPainter *p,const QStyleOptionViewItem &opt,
			   const QModelIndex &idx) const override;

private:
	static constexpr int kHMargin   = 10;
	static constexpr int kPadding   = 12;  // Increased for better appearance
	static constexpr int kVPadding  = 10;  // Increased for better appearance
	static constexpr int kRadius    = 12;
	static constexpr int kMinBubble = 120;
};