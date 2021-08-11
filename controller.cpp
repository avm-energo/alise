#include "controller.h"

#include "../gen/datamanager.h"
#include "../gen/stdfunc.h"

#include <iostream>

constexpr int minSecs = 60;
constexpr int hourSecs = minSecs * minSecs;

Controller::Controller(QObject *parent) noexcept : Controller("0.0.0.0", parent)
{
}

Controller::Controller(std::string addr, QObject *parent) noexcept
    : QObject(parent), worker(new runner::ZeroRunner(this)), m_stmBroker({})

{

    connect(worker, &runner::ZeroRunner::healthReceived, &m_stmBroker, &deviceType::setIndication);
    // NOTE avtuk will be rebooted
    connect(&recovery, &Recovery::rebootReq, &m_stmBroker, &deviceType::rebootMyself);
#if defined(AVTUK_14)
    connect(worker, &runner::ZeroRunner::timeReceived, &m_stmBroker, &deviceType::setTime);
    connect(worker, &runner::ZeroRunner::timeRequest, &m_stmBroker, &deviceType::getTime);

#elif defined(AVTUK_12)
    connect(worker, &runner::ZeroRunner::timeRequest, &timeSync, //
        [&] { DataManager::addSignalToOutList(DataTypes::SignalTypes::Timespec, timeSync.systemTime()); });
#endif
    connect(worker, &runner::ZeroRunner::timeReceived, &timeSync, &TimeSyncronizer::handleTime);

    const auto &manager = DataManager::GetInstance();
    connect(&manager, &DataManager::blockReceived, &recovery, &Recovery::receiveBlock);

    connect(&timeSync, &TimeSyncronizer::ntpStatusChanged, worker, &runner::ZeroRunner::publishNtpStatus);
    connect(&timeSync, &TimeSyncronizer::ntpStatusChanged, this, [&](bool status) {
        if (!status)
            return;
        syncCounter++;
        if (syncCounter == minSecs)
        {
#if defined(AVTUK_14)
            m_stmBroker.setTime(timeSync.systemTime());
#endif
            syncCounter = 0;
        }
    });
}

Controller::~Controller()
{
}

bool Controller::launch()
{
#if defined(AVTUK_14)
    if (!m_stmBroker.connectToStm())
    {
        delete worker;
        return false;
    }
#endif
    const auto &manager = DataManager::GetInstance();
    auto connectionTimeSync = std::shared_ptr<QMetaObject::Connection>(new QMetaObject::Connection);

    *connectionTimeSync = connect(
        &manager, &DataManager::timeReceived, &timeSync,
        [=, &syncer = timeSync](const timespec &time) {
            QObject::disconnect(*connectionTimeSync);
            syncer.handleTime(time);
        },
        Qt::DirectConnection);
#if defined(AVTUK_14)
    m_stmBroker.getTime();
#endif
    worker->runServer();

    return true;
}

void Controller::shutdown()
{
    QMetaObject::invokeMethod(worker, &runner::ZeroRunner::stopServer, Qt::DirectConnection);
}

void Controller::syncTime(const timespec &)
{
}
