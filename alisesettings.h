#ifndef ALISESETTINGS_H
#define ALISESETTINGS_H

#include <QMap>
#include <QSettings>

class AliseSettings
{
public:
    AliseSettings();

    void init();
    void readSettings();
    void logSettings();
    void writeSetting(); // write module properties: serialnum, hwversion, etc
    bool isModuleInfoFilled();
    QString versionStr(const uint32_t &version);

    QString logLevel;
    QString logFilename;
    std::uint32_t serialNum;
    std::uint32_t serialNumB;
    std::uint32_t hwVersion;
    std::uint32_t swVersion;
    int portCore;
    int portBooter;
    int portAdminja;

private:
    QSettings *m_settings;
};

#endif // ALISESETTINGS_H
