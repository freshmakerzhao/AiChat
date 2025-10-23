#pragma once
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QTextBrowser>
#include <QTimer>
#include <QListWidgetItem>
#include <QListWidget>
#include <QTextDocument>
#include <QTextOption>
#include <QDebug>

class AiBubbleWidget: public QWidget {
	Q_OBJECT
public:
	explicit AiBubbleWidget(QWidget *parent=nullptr);
	
	void setMarkdownText(const QString &md);
	void updateMarkdownTextStreaming(const QString &md);
	void setViewportWidth(int vw);
	void setHostItem(QListWidgetItem *it);
	
	QSize sizeHint() const override;

private slots:
	void performRecalc();

protected:
	void resizeEvent(QResizeEvent *e) override;

private:
	void calculateSizeHint();
	int calcBubbleWidth() const;
	void ensureMeasured() const;

private:
	// Constants
	QString rawMd;  // Save original content passed to setMarkdownText
	static constexpr int kHMargin   = 10;  // External margin
	static constexpr int kPadding   = 12;  // Bubble internal left/right margin
	static constexpr int kVPadding  = 10;  // Bubble internal top/bottom margin
	static constexpr int kMinBubble = 120; // Minimum bubble width

private:
	// Structure
	QHBoxLayout *outerLay = nullptr;
	QVBoxLayout *innerLay = nullptr;
	QFrame      *bubble   = nullptr;
	QTextBrowser *browser = nullptr;
	QTimer      *recalcTimer = nullptr;

	int viewW = 0;
	QListWidgetItem *hostItem = nullptr;

	// Cache
	QSize cachedSizeHint{200,40};
	bool  measured = false;
	int   cachedContentHeight = 0;
};