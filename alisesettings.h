#ifndef ALISESETTINGS_H
#define ALISESETTINGS_H

#include <QMap>
#include <QSettings>
#include <gen/logger.h>

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

    QString logLevel;
    QString logFilename;
    std::uint32_t serialNum;
    std::uint32_t serialNumB;
    std::uint32_t hwVersion;
    std::uint32_t swVersion;
    int httpPort;
    QMap<QString, GPIOInfo> m_gpioMap;
    bool gpioExceptionsAreOn;

private:
    QSettings *m_settings;
    bool m_isValid;
};

#endif // ALISESETTINGS_H
