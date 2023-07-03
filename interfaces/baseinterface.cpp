#include "baseinterface.h"

#include "baseport.h"

#include <QCoreApplication>
#include <QMutexLocker>
#include <QTimer>
#include <gen/stdfunc.h>
#include <memory>

namespace Interface
{

// Static members
BaseInterface::InterfacePointer BaseInterface::m_iface;

BaseInterface::BaseInterface(QObject *parent) : QObject(parent) /* m_working(false),*/, ifacePort(nullptr)
{
    ProxyInit();
    qRegisterMetaType<State>();
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setInterval(MAINTIMEOUT);
    connect(m_timeoutTimer, &QTimer::timeout, this, &BaseInterface::timeout);
    m_state = State::Connect;
}

void BaseInterface::ProxyInit()
{
    proxyBS = UniquePointer<DataTypesProxy>(new DataTypesProxy());
    proxyGRS = UniquePointer<DataTypesProxy>(new DataTypesProxy());

    proxyBS->RegisterType<DataTypes::BlockStruct>();
    proxyGRS->RegisterType<DataTypes::GeneralResponseStruct>();

#ifdef __linux__
    proxyTS = UniquePointer<DataTypesProxy>(new DataTypesProxy(&DataManager::GetInstance()));
    proxyTS->RegisterType<timespec>();
#endif
}

void BaseInterface::reqTime()
{
    CommandStruct bi { C_ReqTime, 0, 0 };
    setToQueue(bi);
}

void BaseInterface::writeTime(quint32 time)
{
    CommandStruct bi { C_WriteTime, time, 0 };
    setToQueue(bi);
}

void BaseInterface::writeTime(const timespec &time)
{
    CommandStruct bi { C_WriteTime, QVariant::fromValue(time), QVariant() };
    setToQueue(bi);
}
void BaseInterface::writeCommand(Commands cmd, QVariant value)
{
    CommandStruct bi { cmd, value, QVariant() };
    setToQueue(bi);
}

void BaseInterface::resultReady(const QVariant &msg)
{
    auto result = msg.value<DataTypes::BlockStruct>();
    disconnect(proxyBS.get(), &DataTypesProxy::DataStorable, this, &BaseInterface::resultReady);
    m_byteArrayResult = result.data;
    m_busy = false;
}

void BaseInterface::responseReceived(const QVariant &msg)
{
    auto response = msg.value<DataTypes::GeneralResponseStruct>();
    if ((response.type == DataTypes::GeneralResponseTypes::DataSize)
        || (response.type == DataTypes::GeneralResponseTypes::DataCount))
        return;
    disconnect(proxyGRS.get(), &DataTypesProxy::DataStorable, this, &BaseInterface::responseReceived);
    m_responseResult = (response.type == DataTypes::GeneralResponseTypes::Ok);
    m_busy = false;
}

void BaseInterface::timeout()
{
    m_busy = false;
}

void BaseInterface::setToQueue(CommandStruct &cmd)
{
    DataManager::GetInstance().addToInQueue(cmd);
    emit wakeUpParser();
}

State BaseInterface::state()
{
    QMutexLocker locker(&_stateMutex);
    return m_state;
}

void BaseInterface::setState(const State &state)
{
    QMutexLocker locker(&_stateMutex);
    m_state = state;
    emit stateChanged(m_state);
}

void BaseInterface::close()
{
    DataManager::GetInstance().clearQueue();
    if (ifacePort)
        ifacePort->closeConnection();
}

} // namespace Interface
