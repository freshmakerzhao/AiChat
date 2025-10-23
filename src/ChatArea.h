#pragma once
#include <QScrollArea>
#include "ChatBubble.h">
#include <QWidget>
#include <QVBoxLayout>

class ChatArea: public QScrollArea {
	Q_OBJECT
public:
	explicit ChatArea(QWidget *parent = nullptr);

	ChatBubble* addBubble(bool isUser);

protected:
	void resizeEvent(QResizeEvent *ev) override;

private slots:
	void onBubbleNeedScroll();

private:
	int calcContentWidth() const;

	QWidget *m_container;
	QVBoxLayout *m_layout;
};