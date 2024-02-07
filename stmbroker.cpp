#include "stmbroker.h"

#include "aliseconstants.h"
#include "controller.h"
#include "helper.h"
#include "interfaces/protocom.h"
#include "interfaces/usbhidport.h"
#include "timesyncronizer.h"

#include <QRandomGenerator>
#include <gen/helper.h>

StmBroker::StmBroker(QObject *parent) : Broker(parent)
{
}

bool StmBroker::connect()
{
#ifndef ALISE_LOCALDEBUG
    const auto devices = UsbHidPort::devicesFound(0x0483);
    if (devices.isEmpty())
    {
        std::cout << "No devices" << std::endl;
        return false;
    }
    for (const auto &device : devices)
    {
        std::cout << "Vendor id:" << std::hex << device.vendor_id << std::dec << " : "
                  << "Product id:" << std::hex << device.product_id << std::dec << " : "
                  << "Serial number:" << device.serial.toStdString() << std::endl;
    }
    BaseInterface::InterfacePointer device;
    device = BaseInterface::InterfacePointer(new Protocom());
    BaseInterface::setIface(std::move(device));

    m_interface = static_cast<Protocom *>(BaseInterface::iface());
    if (!m_interface->start(devices.first()))
    {
        std::cout << "Couldn't connect" << std::endl;
        return false;
    }

#endif
    return true;
}

void StmBroker::checkPowerUnit()
{
#ifndef ALISE_LOCALDEBUG
    QMutexLocker locker(&_mutex);
    Q_CHECK_PTR(m_interface);
    m_interface->writeCommand(Interface::Commands::C_ReqBlkData, AVTUK_CCU::MainBlock);
#endif
}

void StmBroker::checkIndication()
{
#ifndef ALISE_LOCALDEBUG
    QMutexLocker locker(&_mutex);
    Q_CHECK_PTR(m_interface);
    m_interface->writeCommand(Interface::Commands::C_ReqBlkData, AVTUK_CCU::IndicationBlock);
#endif
}

void StmBroker::setIndication(const AVTUK_CCU::Indication &indication)
{
    QMutexLocker locker(&_mutex);
    if (m_currentIndication == indication)
        return;
    qDebug() << "Indication is: cnt1: " << indication.PulseCnt1 << ", freq1: " << indication.PulseFreq1
             << ", cnt2: " << indication.PulseCnt2 << ", freq2: " << indication.PulseFreq2;
    m_currentIndication.PulseCnt1 = indication.PulseCnt1;
    m_currentIndication.PulseCnt2 = indication.PulseCnt2;
    m_currentIndication.PulseFreq1 = Alise::AliseConstants::freqByPeriod(indication.PulseFreq1);
    m_currentIndication.PulseFreq2 = Alise::AliseConstants::freqByPeriod(indication.PulseFreq2);
#ifndef ALISE_LOCALDEBUG
    DataTypes::BlockStruct block;
    block.ID = AVTUK_CCU::IndicationBlock;
    block.data.resize(sizeof(indication));
    memcpy(block.data.data(), &indication, sizeof(indication));
    m_interface->writeCommand(Interface::Commands::C_WriteUserValues, QVariant::fromValue(block));
#endif
}

void StmBroker::setTime(timespec time)
{
#ifndef ALISE_LOCALDEBUG
    QMutexLocker locker(&_mutex);
    Q_CHECK_PTR(m_interface);
    m_interface->writeTime(time);
#endif
}

void StmBroker::getTime()
{
#ifndef ALISE_LOCALDEBUG
    QMutexLocker locker(&_mutex);
    Q_CHECK_PTR(m_interface);
    m_interface->reqTime();
#endif
}

void StmBroker::rebootMyself()
{
#ifndef ALISE_LOCALDEBUG
    m_interface->writeCommand(Interface::Commands::C_Reboot, 0xff);
#endif
}

void StmBroker::currentIndicationReceived(const QVariant &msg)
{
    auto blk = msg.value<DataTypes::BlockStruct>();
    qDebug() << "[StmBroker] <= MCU : Block ID = " << blk.ID << ", data = " << blk.data;
    if (blk.ID == AVTUK_CCU::IndicationBlock)
        memcpy(&m_currentIndication, blk.data.data(), sizeof(m_currentIndication));
}

timespec StmBroker::transform(google::protobuf::Timestamp timestamp) const
{
    timespec temp;
    temp.tv_nsec = timestamp.nanos();
    temp.tv_sec = timestamp.seconds();
    return temp;
}
