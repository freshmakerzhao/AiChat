#pragma once
#include <QWidget>
#include <QString>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QTimer>

class ChatBubble: public QWidget {
	Q_OBJECT
public:
	explicit ChatBubble(QWidget *parent = nullptr);
	~ChatBubble() override;

	// 设置内容最大宽度（content area，不含内边距）
	void setMaxContentWidth(int w);

public slots:
	// 在 GUI 线程直接调用；若从 worker 线程发信号，请使用 Qt::QueuedConnection
	void appendStreamChunk(const QString &chunk);
	// 标记流结束并强制立即刷新
	void finalize();

signals:
	// 建议外层容器连接此信号以滚到底部
	void needScrollToBottom();

protected:
	QSize sizeHint() const override;

private slots:
	void applyPending();

private:
	void scheduleApply();

private:
	QTextBrowser *m_browser = nullptr;
	QVBoxLayout  *m_layout = nullptr;
	QTimer       *m_timer = nullptr;

	QString m_raw;
	int     m_maxContentWidth = 0;
	int     m_cachedHeight = 40;
};