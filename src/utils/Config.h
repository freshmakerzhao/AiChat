#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>

namespace Config {

    // 从 config.env 文件读取配置值
    inline QString getValue(const QString &key, const QString &defaultValue = QString())
    {
        static QMap<QString, QString> configCache;
        static bool loaded = false;
        
        if (!loaded) {
            // 尝试从不同位置加载配置文件
            QStringList configPaths = {
        		":/resource/config.env"
            };
            
            bool configFound = false;
            for (const QString &path : configPaths) {
                QFile file(path);
                if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&file);
                    while (!in.atEnd()) {
                        QString line = in.readLine().trimmed();
                        if (line.isEmpty() || line.startsWith('#')) {
                            continue; // 跳过空行和注释
                        }
                        
                        int equalPos = line.indexOf('=');
                        if (equalPos > 0) {
                            QString k = line.left(equalPos).trimmed();
                            QString v = line.mid(equalPos + 1).trimmed();
                            configCache[k] = v;
                        }
                    }
                    configFound = true;
                    qDebug() << "[Config] 已加载配置文件:" << path;
                    break;
                }
            }
            
            if (!configFound) {
                qWarning() << "[Config] 未找到配置文件，请确保 config.env 存在";
            }
            loaded = true;
        }
        
        return configCache.value(key, defaultValue);
    }

    // 常用配置项的便捷访问方法
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