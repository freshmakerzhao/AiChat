# AI Chat - Qt DeepSeek 聊天应用

一个使用 Qt 开发的现代化 AI 聊天应用，集成 DeepSeek API，支持流式回复显示和 Markdown 渲染。

## ✨ 功能特点

- 🎯 **流式回复显示** - 模拟 ChatGPT 的逐字符显示效果
- 📝 **Markdown 支持** - 完整支持 Markdown 格式渲染
- 🎨 **现代化界面** - 左右气泡式对话界面，支持窗口自适应
- 🌐 **DeepSeek 集成** - 使用 OpenAI 兼容的 API 格式
- ⚡ **丝滑滚动** - 像素级平滑滚动体验

## 🚀 快速开始

### 1. 获取 API Key

1. 访问 [DeepSeek Platform](https://platform.deepseek.com/)
2. 注册账号并登录
3. 在控制台中创建 API Key
4. 复制您的 API Key

### 2. 配置项目

1. 打开项目根目录下的 `config.env` 文件
2. 将您的 API Key 替换默认值：
   ```env
   DEEPSEEK_API_KEY=sk-your-actual-api-key-here
   ```
3. 可选：调整其他配置参数（模型、温度等）

### 3. 编译运行

```bash
# 使用 CMake 编译
cd qt5_component_AI_chat
mkdir build && cd build
cmake ..
make

# 或者使用 Qt Creator 直接打开项目文件
```

## 📁 项目结构

```
src/
├── main.cpp                 # 程序入口
├── mainwindow.h/cpp         # 主窗口（聊天界面）
├── utils/
│   ├── Config.h            # 配置文件读取工具
│   ├── markdownUtil.h/cpp  # Markdown 渲染工具
│   └── FontsUtil.h         # 字体加载工具
└── resource/               # 资源文件
config.env                  # 配置文件
```

## ⚙️ 配置说明

配置文件 `config.env` 中的参数：

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `DEEPSEEK_API_KEY` | DeepSeek API 密钥 | 必须配置 |
| `DEEPSEEK_API_URL` | API 端点 URL | `https://api.deepseek.com/v1/chat/completions` |
| `MODEL_NAME` | 使用的模型名称 | `deepseek-chat` |
| `MAX_TOKENS` | 最大回复长度 | `2000` |
| `TEMPERATURE` | 创造性参数 (0-1) | `0.7` |

## 🎮 使用方法

1. **发送消息**：在底部输入框输入文本，按 Enter 或点击"send"按钮
2. **换行**：使用 Shift+Enter 在输入框中换行
3. **查看回复**：AI 回复会在左侧气泡中流式显示
4. **Markdown 支持**：AI 回复支持标题、代码块、列表、引用等格式

## 🛠️ 技术实现

### 核心技术栈
- **Qt 5.x** - 跨平台 GUI 框架
- **QNetworkAccessManager** - HTTP 网络请求
- **QTimer** - 流式显示定时器
- **md4c** - Markdown 渲染引擎

### 关键特性
- **自定义气泡组件** (`AiBubbleWidget`) - 支持宽度自适应和高度动态计算
- **流式文本显示** - 使用定时器模拟逐字符显示，在标点符号处适当停顿
- **网络请求管理** - 支持请求取消、错误处理和重试机制
- **配置管理系统** - 从文件读取配置，支持多路径查找

## 🔧 开发说明

### 添加新功能
1. 网络请求相关代码在 `MainWindow::sendApiRequest()` 和 `MainWindow::onApiResponse()`
2. 流式显示逻辑在 `MainWindow::startStreamResponse()` 和 `MainWindow::onStreamTimer()`
3. UI 组件主要在 `AiBubbleWidget` 和 `BubbleDelegate` 中

### 自定义配置
- 修改 `src/utils/Config.h` 来添加新的配置项
- 在 `config.env` 中添加对应的键值对

## 📝 注意事项

1. **API Key 安全**：请勿将 API Key 提交到版本控制系统
2. **网络环境**：确保网络能访问 DeepSeek API
3. **配置文件位置**：程序会自动搜索配置文件，支持多个路径

## 🤝 贡献

欢迎提交 Issue 和 Pull Request 来改进这个项目！

## 📄 许可证

本项目使用 MIT 许可证。