#pragma once
#include <QString>

namespace MarkdownUtil {

    // Convert Markdown to HTML page that can be used by QTextBrowser.
    // - basePointSize: base font size for outer <body> (pt)
    // - disableHtml  : when true, disable native HTML in Markdown source (secure display)
    QString markdownToHtml(const QString& md,
        int basePointSize = 10,
        bool disableHtml = false);

} // namespace MarkdownUtil
