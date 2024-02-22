#ifndef MAINCREATOR_H
#define MAINCREATOR_H

#include "broker.h"
#include "recovery.h"
#include "timesyncronizer.h"

#include <QObject>

class MainCreator : public QObject
{
public:
    MainCreator(QObject *parent = nullptr);

    Broker *create(bool &ok);
    TimeSyncronizer *getTimeSynchronizer();

private:
    TimeSyncronizer *m_timeSynchronizer;
    RecoveryEngine m_recoveryEngine;
};

#endif // MAINCREATOR_H
