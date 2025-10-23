#include <QApplication>
#include "mainwindow.h"
#include <utils/FontsUtil.h>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    //! ----- Fonts -----
    FontsUtil::loadFont();

    MainWindow w;   // 创建主窗口
    w.show();       // 显示主窗口

    return app.exec(); // 进入事件循环
}
