#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include "alisesettings.h"

#include <QObject>

#ifdef AVTUK_STM
#include "stmbroker.h"
#endif

class CommandLineParser : public QObject
{
    Q_OBJECT
public:
    CommandLineParser(QObject *parent = nullptr);

    bool parseCommandLine(AliseSettings &settings);

private:
    void setSerialNumber(const QString &serialNum);
    void setSerialNumberB(const QString &serialNumB);
    void setHWVersion(const QString &hwversion);

#ifdef AVTUK_NO_STM
    void listPins();
#else
    StmBroker *m_broker;
    void waitForBSIOrTimeout();
    void writeHiddenBlock();

#endif
};

#endif // COMMANDLINEPARSER_H
