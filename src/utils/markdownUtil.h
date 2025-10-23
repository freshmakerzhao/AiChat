#pragma once
#include <QString>

namespace MarkdownUtil {

    // 将 Markdown 转为可给 QTextBrowser 使用的 HTML 页面。
    // - basePointSize: 外层 <body> 的基础字号（pt）
    // - disableHtml  : 为 true 时，屏蔽 Markdown 源中的原生 HTML（安全展示）
    QString markdownToHtml(const QString& md,
        int basePointSize = 10,
        bool disableHtml = false);

} // namespace MarkdownUtil
