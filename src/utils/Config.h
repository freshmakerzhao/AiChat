#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QMap>

namespace Config {

    // Simple cached configuration reading
    inline QString getValue(const QString &key, const QString &defaultValue = QString())
    {
        static QMap<QString, QString> configCache;
        static bool loaded = false;
        
        if (!loaded) {
            // Try to load config file from multiple locations
            QStringList configPaths = {
                QDir::currentPath() + "/config.env",
                ":/resource/config.env"
            };
            
            for (const QString &path : configPaths) {
                QFile file(path);
                if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&file);
                    while (!in.atEnd()) {
                        QString line = in.readLine().trimmed();
                        if (line.isEmpty() || line.startsWith('#')) {
                            continue;
                        }
                        
                        int equalPos = line.indexOf('=');
                        if (equalPos > 0) {
                            QString k = line.left(equalPos).trimmed();
                            QString v = line.mid(equalPos + 1).trimmed();
                            configCache[k] = v;
                        }
                    }
                    qDebug() << "[Config] Loaded configuration from:" << path;
                    break;
                }
            }
            loaded = true;
        }
        
        return configCache.value(key, defaultValue);
    }

    // Convenience methods
    inline QString getDeepSeekApiKey() {
        return getValue("DEEPSEEK_API_KEY");
    }
    
    inline QString getDeepSeekApiUrl() {
        return getValue("DEEPSEEK_API_URL", "https://api.deepseek.com/v1/chat/completions");
    }
    
    inline QString getModelName() {
        return getValue("MODEL_NAME", "deepseek-chat");
    }
    
    inline int getMaxTokens() {
        return getValue("MAX_TOKENS", "2000").toInt();
    }
    
    inline double getTemperature() {
        return getValue("TEMPERATURE", "0.7").toDouble();
    }

} // namespace Config

#endif // CONFIG_H