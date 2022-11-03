#pragma once
#include "recovery.h"
#include "timesyncronizer.h"
#include "zerorunner.h"

// clang-format off
#if defined(AVTUK_STM)
    #include "stmbroker.h"
    using deviceType = StmBroker;
#elif defined(AVTUK_NO_STM)
    #include "gpiobroker.h"
    using deviceType = GpioBroker;
#else
    using deviceType = void;
#endif
// clang-format on

#include <QObject>
#include <QThread>
#include <gen/datamanager/typesproxy.h>

class Controller : public QObject
{
public:
    explicit Controller(deviceType *devBroker, QObject *parent = nullptr) noexcept;
    explicit Controller(std::string addr, deviceType *devBroker, QObject *parent = nullptr) noexcept;

    ~Controller() override;

    bool launch(int port);
    void shutdown();
    void syncTime(const timespec &);
signals:

private:
    runner::ZeroRunner *worker;
    deviceType *deviceBroker;
    TimeSyncronizer timeSync;
    Recovery recovery;
    int syncCounter = 0;
    UniquePointer<DataTypesProxy> proxyBS, proxyTS;
};
