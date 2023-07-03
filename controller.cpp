#include "controller.h"

#include <iostream>

constexpr int minSecs = 60;

Controller::Controller(deviceType *devBroker, QObject *parent) noexcept : Controller("0.0.0.0", devBroker, parent)
{
}

Controller::Controller(std::string addr, deviceType *devBroker, QObject *parent) noexcept
    : QObject(parent), worker(new runner::ZeroRunner(this)), deviceBroker(devBroker)
{
    Q_UNUSED(addr);

    connect(worker, &runner::ZeroRunner::healthReceived, deviceBroker, &deviceType::setIndication);
    // NOTE avtuk will be rebooted
    connect(&recovery, &Recovery::rebootReq, deviceBroker, &deviceType::rebootMyself);

#if defined(AVTUK_STM)
    connect(worker, &runner::ZeroRunner::timeReceived, deviceBroker, &deviceType::setTime);
    connect(worker, &runner::ZeroRunner::timeRequest, deviceBroker, &deviceType::getTime);
#elif defined(AVTUK_NO_STM)
    connect(worker, &runner::ZeroRunner::timeRequest, &timeSync, //
        [&] {
            auto ts = timeSync.systemTime();
            DataManager::GetInstance().addSignalToOutList(ts);
        });
#endif

    connect(worker, &runner::ZeroRunner::timeReceived, &timeSync, &TimeSyncronizer::handleTime);
    connect(proxyBS.get(), &DataTypesProxy::DataStorable, &recovery, &Recovery::receiveBlock);

    connect(&timeSync, &TimeSyncronizer::ntpStatusChanged, worker, &runner::ZeroRunner::publishNtpStatus);
    connect(&timeSync, &TimeSyncronizer::ntpStatusChanged, this, [&](bool status) {
        if (!status)
            return;
        syncCounter++;
        if (syncCounter == minSecs)
        {
#if defined(AVTUK_STM)
            deviceBroker->setTime(timeSync.systemTime());
#endif
            syncCounter = 0;
        }
    });
}

Controller::~Controller()
{
}

bool Controller::launch(int port)
{
#if defined(AVTUK_STM)
    if (!deviceBroker->connectToStm())
    {
        delete worker;
        return false;
    }
#endif
    auto connectionTimeSync = std::shared_ptr<QMetaObject::Connection>(new QMetaObject::Connection);
    *connectionTimeSync = connect(
        proxyTS.get(), &DataTypesProxy::DataStorable, &timeSync,
        // [=, &syncer = timeSync](const timespec &time) {
        [=, &syncer = timeSync](const QVariant &msg) {
            auto time = msg.value<timespec>();
            QObject::disconnect(*connectionTimeSync);
            syncer.handleTime(time);
        },
        Qt::DirectConnection);

#if defined(AVTUK_STM)
    deviceBroker->getTime();
#elif defined(AVTUK_NO_STM)
    timeSync.systemTime();
#endif

    worker->runServer(port);
    return true;
}

void Controller::shutdown()
{
    QMetaObject::invokeMethod(worker, &runner::ZeroRunner::stopServer, Qt::DirectConnection);
}

void Controller::syncTime(const timespec &)
{
}
