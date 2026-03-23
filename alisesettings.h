#pragma once

#include <QMap>
#include <QSettings>
#include <avm-gen/logger.h>

class AliseSettings
{
public:
    struct GPIOInfo
    {
        int pin;
        int offset;
    };

    AliseSettings();

    void init();
    void readSettings();
    void logSettings();
    void writeSettings(); // write all settings
    void flush();
    bool isValid();
    GPIOInfo parseGPIOSettings(const QString &pinName);
    void setGPIOValues();

    QString m_logLevel;
    QString logFilename;
    std::uint32_t m_serialNum;
    std::uint32_t m_serialNumB;
    std::uint32_t m_hwVersion;
    std::uint32_t m_swVersion;
    int m_httpPort;
    QMap<QString, GPIOInfo> m_gpioMap;
    bool m_gpioExceptionsAreOn;
    Logger m_log;

private:
    QSettings *m_settings;
    bool m_isValid;
};
