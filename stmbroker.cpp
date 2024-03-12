#include "stmbroker.h"

#include "aliseconstants.h"

#include <QRandomGenerator>
#include <interfaces/connectionmanager.h>
#include <interfaces/types/usbhidportinfo.h>

StmBroker::StmBroker(QObject *parent)
    : Broker(parent), m_manager(new Interface::ConnectionManager(this)), m_conn(nullptr)
{
}

bool StmBroker::connect()
{
#ifndef ALISE_LOCALDEBUG
    const auto devices = UsbHidPortInfo::devicesFound(0x0483);
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

    UsbHidSettings settings { devices.first() };
    settings.m_timeout = 1000;
    settings.m_maxErrors = 5;
    settings.m_maxTimeouts = 5;
    settings.m_isLoggingEnabled = true;
    settings.m_reconnectInterval = 100;
    settings.m_silentInterval = 100;
    m_conn = m_manager->createConnection(settings);
    if (m_conn == nullptr)
    {
        std::cout << "Couldn't connect" << std::endl;
        return false;
    }
    else
    {
        m_conn->connection(this, &StmBroker::currentIndicationReceived);
        m_conn->connection(static_cast<Broker *>(this), &Broker::updateBlock);
        m_conn->connection(static_cast<Broker *>(this), &StmBroker::updateTime);
        m_conn->connection(this, &StmBroker::updateBsi);
        m_conn->writeCommand(Interface::Commands::C_ReqBSI);
        return true;
    }
#else
    return true;
#endif
}

void StmBroker::writeHiddenBlock()
{
#ifndef ALISE_LOCALDEBUG
    QMutexLocker locker(&_mutex);
    Q_CHECK_PTR(m_conn);
    DataTypes::BlockStruct block;
    block.ID = 0x01; // base block
    block.data.resize(sizeof(Alise::AliseConstants::ModuleInfo));
    memcpy(block.data.data(), &Alise::AliseConstants::s_moduleInfo, sizeof(Alise::AliseConstants::ModuleInfo));
    m_conn->writeCommand(Interface::Commands::C_WriteHiddenBlock, QVariant::fromValue(block));
#endif
}

void StmBroker::checkPowerUnit()
{
#ifndef ALISE_LOCALDEBUG
    QMutexLocker locker(&_mutex);
    Q_CHECK_PTR(m_conn);
    m_conn->writeCommand(Interface::Commands::C_ReqBlkData, AVTUK_CCU::MainBlock);
#endif
}

void StmBroker::checkIndication()
{
#ifndef ALISE_LOCALDEBUG
    QMutexLocker locker(&_mutex);
    Q_CHECK_PTR(m_conn);
    m_conn->writeCommand(Interface::Commands::C_ReqBlkData, AVTUK_CCU::IndicationBlock);
#endif
}

void StmBroker::setIndication(const AVTUK_CCU::Indication &indication)
{
    QMutexLocker locker(&_mutex);
    if (m_currentIndication == indication)
        return;
    m_currentIndication.PulseCnt1 = indication.PulseCnt1;
    m_currentIndication.PulseCnt2 = indication.PulseCnt2;
    m_currentIndication.PulseFreq1 = Alise::AliseConstants::freqByPeriod(indication.PulseFreq1);
    m_currentIndication.PulseFreq2 = Alise::AliseConstants::freqByPeriod(indication.PulseFreq2);
    qDebug() << "Indication is: cnt1: " << indication.PulseCnt1 << ", freq1: " << indication.PulseFreq1
             << ", cnt2: " << indication.PulseCnt2 << ", freq2: " << indication.PulseFreq2;
#ifndef ALISE_LOCALDEBUG
    DataTypes::BlockStruct block;
    block.ID = AVTUK_CCU::IndicationBlock;
    block.data.resize(sizeof(m_currentIndication));
    memcpy(block.data.data(), &m_currentIndication, sizeof(m_currentIndication));
    m_conn->writeCommand(Interface::Commands::C_WriteUserValues, QVariant::fromValue(block));
#endif
}

void StmBroker::setTime(const timespec &time)
{
#ifndef ALISE_LOCALDEBUG
    QMutexLocker locker(&_mutex);
    Q_CHECK_PTR(m_conn);
    m_conn->writeTime(time);
#endif
}

void StmBroker::getTime()
{
#ifndef ALISE_LOCALDEBUG
    QMutexLocker locker(&_mutex);
    Q_CHECK_PTR(m_conn);
    m_conn->reqTime();
#endif
}

void StmBroker::rebootMyself()
{
#ifndef ALISE_LOCALDEBUG
    m_conn->writeCommand(Interface::Commands::C_Reboot, 0xff);
#endif
}

void StmBroker::currentIndicationReceived(const DataTypes::BlockStruct &blk)
{
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

void StmBroker::updateBsi(const DataTypes::BitStringStruct &resp)
{
    switch (resp.sigAdr)
    {
    case ModuleSerialNumAdr:
        m_settings.serialNum = resp.sigVal;
        break;
    case SerialNumBAdr:
        m_settings.serialNumB = resp.sigVal;
        break;
    case HWAdr:
        m_settings.hwVersion = resp.sigVal;
        break;
    case SWAdr:
        m_settings.swVersion = resp.sigVal;
        break;
    }
    if (m_settings.isModuleInfoFilled())
    {
        Alise::AliseConstants::s_moduleInfo.ModuleSerialNumber = m_settings.serialNum;
        Alise::AliseConstants::s_moduleInfo.SerialNumber = m_settings.serialNumB;
        Alise::AliseConstants::s_moduleInfo.HWVersion = m_settings.hwVersion;
        m_settings.writeSetting();
        emit ModuleInfoFilled();
    }
}
