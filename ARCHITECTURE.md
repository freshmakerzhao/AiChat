# AI Chat Widget - Modular Architecture

A highly optimized, embeddable Qt AI chat widget with real-time streaming support.

## 🏗️ Project Structure

```
src/
├── main.cpp                    # Application entry point
├── mainwindow.h/cpp           # Main window (minimal wrapper)
├── widgets/                   # UI Components
│   ├── AiChatWidget.h/cpp     # Main chat widget (embeddable)
│   ├── AiBubbleWidget.h/cpp   # AI message bubble
│   └── BubbleDelegate.h/cpp   # User message delegate
├── core/                      # Business Logic
│   └── ChatManager.h/cpp      # Chat request management
├── network/                   # Network Layer
│   └── NetworkManager.h/cpp   # HTTP streaming handler
├── utils/                     # Utilities
│   ├── Config.h/cpp          # Configuration management
│   ├── PerformanceOptimizer.h/cpp # Performance utilities
│   ├── markdownUtil.h/cpp    # Markdown rendering
│   └── FontsUtil.h           # Font management
└── resource/                  # Resources
    ├── config.env            # Configuration file
    └── fonts/               # Font files
```

## 🎯 Key Improvements

### 1. Fixed Bubble Overlap Issue
- Increased bottom margins from 12px to 20px
- Enhanced spacing between list items
- Better size hint calculations

### 2. Modular Architecture for Embedding
- **AiChatWidget**: Self-contained chat component
- **Separate UI/Business Logic**: Clean separation of concerns
- **Embeddable Design**: Easy integration into larger applications

### 3. Performance Optimizations
- **PerformanceOptimizer**: Debouncing, throttling, and batching
- **Smart Layout Updates**: Reduced layout thrashing
- **Memory Management**: Automatic garbage collection
- **Rate-Limited Scrolling**: Prevents excessive updates (10 FPS limit)

## 🔧 Usage as Embedded Widget

### Basic Integration
```cpp
#include "widgets/AiChatWidget.h"

// Create chat widget
AiChatWidget* chatWidget = new AiChatWidget(parentWidget);

// Configure API
chatWidget->setApiConfiguration(apiKey, apiUrl, modelName);

// Connect signals
connect(chatWidget, &AiChatWidget::messageAdded, 
        this, &YourClass::onMessageAdded);
connect(chatWidget, &AiChatWidget::errorOccurred, 
        this, &YourClass::onError);

// Add to your layout
layout->addWidget(chatWidget);
```

### Advanced Usage
```cpp
// Programmatic message sending
chatWidget->sendMessage("Hello AI!");

// Add messages without API calls
chatWidget->addMessage("System message", false);

// Clear chat history
chatWidget->clearChat();

// Monitor API requests
connect(chatWidget, &AiChatWidget::apiRequestStarted, 
        this, &YourClass::showProgressIndicator);
```

## ⚡ Performance Features

### 1. Smart Layout Management
- Debounced recalculation (10ms delay)
- Batched resize events (50ms)
- Queued layout updates to prevent recursion

### 2. Optimized Streaming
- Rate-limited auto-scrolling
- Incremental text updates for small changes
- Efficient buffer management

### 3. Memory Optimization
- Static QTextDocument for calculations
- Automatic cleanup of completed operations
- Reduced widget event handling

## 🎨 Visual Improvements

### AI Bubbles
- Light gray background with subtle borders
- 90% width for better balance
- Increased padding for readability
- Disabled mouse interactions

### User Bubbles
- Blue gradient with shadows
- Dynamic width based on content
- Enhanced anti-aliasing
- Professional appearance

## 📋 Configuration

### Environment Variables (config.env)
```env
DEEPSEEK_API_KEY=your-api-key-here
DEEPSEEK_API_URL=https://api.deepseek.com/v1/chat/completions
MODEL_NAME=deepseek-chat
MAX_TOKENS=2000
TEMPERATURE=0.7
```

### Multiple Config Locations
1. Application directory
2. Resource bundle
3. User config directory
4. Current working directory

## 🔌 Integration Examples

### As Main Widget
```cpp
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    AiChatWidget *chatWidget = new AiChatWidget(this);
    setCentralWidget(chatWidget);
}
```

### As Dock Widget
```cpp
AiChatWidget *chatWidget = new AiChatWidget();
QDockWidget *dock = new QDockWidget("AI Assistant", this);
dock->setWidget(chatWidget);
addDockWidget(Qt::RightDockWidgetArea, dock);
```

### As Dialog
```cpp
QDialog *chatDialog = new QDialog(this);
QVBoxLayout *layout = new QVBoxLayout(chatDialog);
AiChatWidget *chatWidget = new AiChatWidget(chatDialog);
layout->addWidget(chatWidget);
chatDialog->resize(400, 600);
```

## 🚀 Build Instructions

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt5
make -j4
```

## 📝 API Reference

### AiChatWidget Signals
- `messageAdded(const QString &text, bool isUser)`
- `apiRequestStarted()`
- `apiRequestFinished()`
- `errorOccurred(const QString &error)`

### AiChatWidget Methods
- `void addMessage(const QString &text, bool isUser)`
- `void sendMessage(const QString &message)`
- `void clearChat()`
- `void setApiConfiguration(const QString &apiKey, const QString &apiUrl, const QString &model)`

This modular architecture makes the chat widget highly reusable, performant, and easy to integrate into any Qt application.