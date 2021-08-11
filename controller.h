#pragma once
#include "recovery.h"
#if defined(AVTUK_14)
#include "stmbroker.h"
#elif defined(AVTUK_12)
#include "gpiobroker.h"
#endif
#include "timesyncronizer.h"
#include "zerorunner.h"

#include <QObject>
#include <QThread>

class Controller : public QObject
{
public:
#if defined(AVTUK_14)
    using deviceType = StmBroker;
#elif defined(AVTUK_12)
    using deviceType = GpioBroker;
#endif
    explicit Controller(QObject *parent = nullptr) noexcept;
    explicit Controller(std::string addr, QObject *parent = nullptr) noexcept;

    ~Controller() override;

    bool launch();
    void shutdown();
    void syncTime(const timespec &);
signals:

private:
    runner::ZeroRunner *worker;

    deviceType m_stmBroker;

    TimeSyncronizer timeSync;

    Recovery recovery;

    int syncCounter = 0;
};
