#ifndef FONTSUTIL_H
#define FONTSUTIL_H

#include <QApplication>
#include <QFontDatabase>
#include <QDebug>

namespace FontsUtil {

    // 固定资源路径（确保该 ttf 已被加入到 .qrc）
    inline constexpr const char* kFontResPath =
        ":/resource/fonts/LFTEtica/no-liga-LFTEticaMono-Regular-OK.ttf";

    // 返回加载到的字体族名；失败时返回空串
    inline QString loadFixedFontFamily()
    {
        // 先尝试加载固定资源字体
        int id = QFontDatabase::addApplicationFont(kFontResPath);
        if (id == -1) {
            qWarning() << "[FontsUtil] Failed to add font from resource:" << kFontResPath;
            return {};
        }

        const QStringList fams = QFontDatabase::applicationFontFamilies(id);
        if (fams.isEmpty()) {
            qWarning() << "[FontsUtil] No families found in resource font:" << kFontResPath;
            return {};
        }

        const QString family = fams.first();
        qDebug() << "[FontsUtil] Loaded font from resource:" << kFontResPath
            << "family:" << family;
        return family;
    }

    // 加载并设置应用字体；保持当前字号，失败时回退到系统等宽字体
    inline void loadFont()
    {
        // 记录当前字号，避免切换字体改变大小
        const qreal currentPt = QApplication::font().pointSizeF();

        QString family = loadFixedFontFamily();

        QFont appFont;
        if (!family.isEmpty()) {
            appFont = QFont(family);
        }
        else {
            // 回退：系统等宽字体
            appFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            qWarning() << "[FontsUtil] Fallback to system fixed font:"
                << appFont.family();
        }

        // 通用等宽/渲染偏好
        appFont.setPointSizeF(currentPt);
        appFont.setStyleHint(QFont::Monospace);
        appFont.setFixedPitch(true);
        appFont.setHintingPreference(QFont::PreferNoHinting); // 减轻中文笔画粘连

        QApplication::setFont(appFont);
        qDebug() << "[FontsUtil] Application font set to:" << appFont.family()
            << "pointSize:" << appFont.pointSizeF();
    }

} // namespace FontsUtil

#endif // FONTSUTIL_H
