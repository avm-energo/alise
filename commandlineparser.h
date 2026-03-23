#pragma once

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
#ifndef AVTUK_STM
    void listPins();
#else
    StmBroker *m_broker;
    void waitForBSIOrTimeout();
    void writeHiddenBlock();

#endif
};
