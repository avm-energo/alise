#pragma once
#include "recovery.h"
#if defined(AVTUK_STM)
#include "stmbroker.h"
#elif defined(AVTUK_NO_STM)
#include "gpiobroker.h"
#endif
#include "../gen/datamanager/typesproxy.h"
#include "timesyncronizer.h"
#include "zerorunner.h"

#include <QObject>
#include <QThread>

class Controller : public QObject
{
public:
#if defined(AVTUK_STM)
    using deviceType = StmBroker;
#elif defined(AVTUK_NO_STM)
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
    UniquePointer<DataTypesProxy> proxyBS, proxyTS;
};
