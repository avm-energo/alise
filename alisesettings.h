#ifndef ALISESETTINGS_H
#define ALISESETTINGS_H

#include <QMap>
#include <QSettings>
#include <gen/logger.h>

class AliseSettings
{
public:
    AliseSettings();

    void init();
    void readSettings();
    void logSettings();
    void writeSettings(); // write all settings
    void flush();

    QString logLevel;
    QString logFilename;
    std::uint32_t serialNum;
    std::uint32_t serialNumB;
    std::uint32_t hwVersion;
    std::uint32_t swVersion;
    int httpPort;
    bool gpioExceptionsAreOn;

private:
    QSettings *m_settings;
};

#endif // ALISESETTINGS_H
