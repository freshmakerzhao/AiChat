#ifndef FONTSUTIL_H
#define FONTSUTIL_H

#include <QApplication>
#include <QFontDatabase>
#include <QDebug>

namespace FontsUtil {

    // Fixed resource path (ensure this ttf has been added to .qrc)
    inline constexpr const char* kFontResPath =
        ":/resource/fonts/LFTEtica/no-liga-LFTEticaMono-Regular-OK.ttf";

    // Returns the loaded font family name; returns empty string on failure
    inline QString loadFixedFontFamily()
    {
        // First try to load fixed resource font
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

    // Load and set application font; maintain current font size, fallback to system monospace font on failure
    inline void loadFont()
    {
        // Record current font size to avoid changing size when switching fonts
        const qreal currentPt = QApplication::font().pointSizeF();

        QString family = loadFixedFontFamily();

        QFont appFont;
        if (!family.isEmpty()) {
            appFont = QFont(family);
        }
        else {
            // Fallback: system monospace font
            appFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            qWarning() << "[FontsUtil] Fallback to system fixed font:"
                << appFont.family();
        }

        // General monospace/rendering preferences
        appFont.setPointSizeF(currentPt);
        appFont.setStyleHint(QFont::Monospace);
        appFont.setFixedPitch(true);
        appFont.setHintingPreference(QFont::PreferNoHinting); // Reduce character overlapping

        QApplication::setFont(appFont);
        qDebug() << "[FontsUtil] Application font set to:" << appFont.family()
            << "pointSize:" << appFont.pointSizeF();
    }

} // namespace FontsUtil

#endif // FONTSUTIL_H
