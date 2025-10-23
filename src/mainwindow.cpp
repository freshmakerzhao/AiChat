#include "mainwindow.h"
#include <utils/Config.h>
#include <QFontMetrics>
#include <QScrollBar>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowTitle(QStringLiteral("AI Chat"));
	resize(520,420);

	central = new QWidget(this);
	auto *mainLay = new QVBoxLayout(central);
	mainLay->setContentsMargins(8,8,8,8);
	mainLay->setSpacing(8);

	list = new QListWidget(central);
	list->setWordWrap(true);
	list->setUniformItemSizes(false);
	list->setSelectionMode(QAbstractItemView::NoSelection);
	list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	list->setFrameShape(QFrame::NoFrame);
	list->setSpacing(12);
	
	// 启用丝滑滚动
	list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	list->verticalScrollBar()->setSingleStep(1);
	
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

	// 初始化网络管理器
	networkManager = new QNetworkAccessManager(this);

	// 初始化流式显示定时器
	streamTimer = new QTimer(this);
	streamTimer->setSingleShot(false);
	connect(streamTimer, &QTimer::timeout, this, &MainWindow::onStreamTimer);

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

	// 发送 API 请求到 DeepSeek
	sendApiRequest(text);
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

void MainWindow::startStreamResponse(const QString &fullText)
{
	// 停止之前的流式显示（如果有）
	if (streamTimer->isActive()) {
		streamTimer->stop();
	}

	// 设置流式显示参数
	currentStreamText = fullText;
	streamPosition = 0;

	// 创建一个空的 AI 气泡
	auto *item = new QListWidgetItem;
	list->addItem(item);
	
	currentStreamWidget = new AiBubbleWidget(list);
	list->setItemWidget(item, currentStreamWidget);
	
	currentStreamWidget->setHostItem(item);
	currentStreamWidget->setViewportWidth(list->viewport()->width());
	currentStreamWidget->setMarkdownText(""); // 开始时为空
	
	list->doItemsLayout();
	list->scrollToBottom();

	// 开始流式显示，每50ms添加一些字符
	streamTimer->start(50);
}

void MainWindow::onStreamTimer()
{
	if (!currentStreamWidget || streamPosition >= currentStreamText.length()) {
		// 流式显示完成
		streamTimer->stop();
		currentStreamWidget = nullptr;
		return;
	}

	// 计算这次要显示的字符数，根据字符类型调整速度
	int charsToAdd = 1;
	QChar currentChar = (streamPosition < currentStreamText.length()) ? 
						currentStreamText.at(streamPosition) : QChar();
	
	// 在标点符号处稍作停顿（降低速度）
	if (currentChar == '.' || currentChar == ',' || currentChar == '!' || 
		currentChar == '?' || currentChar == ';' || currentChar == ':') {
		charsToAdd = 1;
		// 下次间隔稍长一些
		streamTimer->setInterval(150);
	} else if (currentChar == '\n') {
		// 换行符处停顿
		charsToAdd = 1;
		streamTimer->setInterval(100);
	} else {
		// 正常字符，恢复正常速度
		streamTimer->setInterval(50);
		if (streamPosition % 8 == 0) {
			charsToAdd = 2; // 偶尔快一点
		}
	}

	// 确保不会超出文本长度
	charsToAdd = qMin(charsToAdd, currentStreamText.length() - streamPosition);
	
	// 更新显示的文本
	streamPosition += charsToAdd;
	QString partialText = currentStreamText.left(streamPosition);
	
	currentStreamWidget->setMarkdownText(partialText);
	
	// 滚动到底部以跟随新内容
	list->scrollToBottom();
}

void MainWindow::sendApiRequest(const QString &userMessage)
{
	// 如果有正在进行的请求，先取消
	if (currentReply) {
		currentReply->abort();
		currentReply->deleteLater();
		currentReply = nullptr;
	}

	// 从配置获取参数
	QString apiKey = Config::getDeepSeekApiKey();
	if (apiKey.isEmpty() || apiKey == "sk-your-deepseek-api-key-here") {
		startStreamResponse("** configuration error **\n\n please configure the DeepSeek API Key first: \n\n1. Open the 'config.env' file in the project directory \n2. Replace the value of 'DEEPSEEK_API_KEY' \n3 with your API Key. To run the program \n\n** get API Key **: https://platform.deepseek.com/");
		return;
	}

	// 构建 OpenAI 格式的请求 JSON
	QJsonObject json;
	json["model"] = Config::getModelName();
	json["stream"] = false; // 目前使用非流式，后续可改为流式
	
	QJsonArray messages;
	QJsonObject systemMessage;
	systemMessage["role"] = "system";
	systemMessage["content"] = "You are a helpful AI assistant. Please answer all questions in Chinese. Support output in Markdown format.";
	messages.append(systemMessage);
	
	QJsonObject userMsg;
	userMsg["role"] = "user";
	userMsg["content"] = userMessage;
	messages.append(userMsg);
	
	json["messages"] = messages;
	json["max_tokens"] = Config::getMaxTokens();
	json["temperature"] = Config::getTemperature();

	// 创建请求
	QNetworkRequest request;
	request.setUrl(QUrl(Config::getDeepSeekApiUrl()));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());
	request.setRawHeader("User-Agent", "Qt-AI-Chat/1.0");

	// 发送请求
	QJsonDocument doc(json);
	currentReply = networkManager->post(request, doc.toJson());
	
	// 连接响应信号
	connect(currentReply, &QNetworkReply::finished, this, &MainWindow::onApiResponse);
	connect(currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
		[this](QNetworkReply::NetworkError error) {
			qWarning() << "Network error:" << error << currentReply->errorString();
		});

	qDebug() << "API 请求已发送到:" << Config::getDeepSeekApiUrl();
}

void MainWindow::onApiResponse()
{
	if (!currentReply) {
		return;
	}

	// 读取响应数据
	QByteArray responseData = currentReply->readAll();
	QNetworkReply::NetworkError error = currentReply->error();
	int httpStatus = currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	// 清理当前请求
	currentReply->deleteLater();
	currentReply = nullptr;

	if (error != QNetworkReply::NoError) {
		qWarning() << "API 请求失败:" << error << "HTTP状态:" << httpStatus;
		
		// 显示错误信息
		QString errorMsg = QString("error code: %1\nHTTP status : %2\n\n")
						   .arg(error).arg(httpStatus);
		startStreamResponse(errorMsg);
		return;
	}

	// 解析 JSON 响应
	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
	
	if (parseError.error != QJsonParseError::NoError) {
		qWarning() << "JSON 解析失败:" << parseError.errorString();
		startStreamResponse("❌ 响应解析失败: " + parseError.errorString());
		return;
	}

	QJsonObject response = doc.object();
	
	// 检查是否有错误
	if (response.contains("error")) {
		QJsonObject errorObj = response["error"].toObject();
		QString errorMessage = errorObj["message"].toString();
		qWarning() << "API 错误:" << errorMessage;
		startStreamResponse("❌ API 错误: " + errorMessage);
		return;
	}

	// 提取 AI 回复内容
	QString aiReply;
	if (response.contains("choices") && response["choices"].isArray()) {
		QJsonArray choices = response["choices"].toArray();
		if (!choices.isEmpty()) {
			QJsonObject choice = choices[0].toObject();
			if (choice.contains("message")) {
				QJsonObject message = choice["message"].toObject();
				aiReply = message["content"].toString();
			}
		}
	}

	if (aiReply.isEmpty()) {
		qWarning() << "No content was found in the response";
		qDebug() << "[aiReply] full content:" << doc.toJson();
		startStreamResponse("No valid response has been received");
		return;
	}

	qDebug() << "Received the AI reply, length:" << aiReply.length();
	
	// 开始流式显示
	startStreamResponse(aiReply);
}