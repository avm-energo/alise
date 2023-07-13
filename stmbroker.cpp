#include "stmbroker.h"

#include "controller.h"
#include "helper.h"
#include "interfaces/protocom.h"
#include "interfaces/usbhidport.h"
#include "timesyncronizer.h"

#include <QRandomGenerator>
#include <gen/helper.h>

StmBroker::StmBroker(QObject *parent) : QObject(parent)
{
    m_status = false;
    m_currentHealthCode = alise::Health_Code_SettingsError;
}

bool StmBroker::connectToStm()
{
#ifndef ALISE_LOCALDEBUG
    m_status = false;
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

    proxyBS = UniquePointer<DataTypesProxy>(new DataTypesProxy());
    proxyBS->RegisterType<DataTypes::BlockStruct>();
    QObject::connect(proxyBS.get(), &DataTypesProxy::DataStorable, this,
        //[](const auto bs) {
        [](const QVariant &msg) {
            auto bs = msg.value<DataTypes::BlockStruct>();
            qDebug() << bs;
        });

    m_timer.setInterval(1000);

#ifdef TEST_INDICATOR
    m_testTimer.setInterval(10000);
    QObject::connect(&m_testTimer, &QTimer::timeout, this,
        [this] { setIndication(static_cast<alise::Health_Code>(QRandomGenerator::global()->bounded(0, 8))); });
    m_testTimer.start();
#endif

    m_timer.start();
    QObject::connect(&m_timer, &QTimer::timeout, this, &StmBroker::checkPowerUnit);
#endif
    m_status = true;
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

void StmBroker::setIndication(alise::Health_Code code)
{
#ifndef ALISE_LOCALDEBUG
    if (code == m_currentHealthCode)
        return;
    m_currentHealthCode = code;
    QMutexLocker locker(&_mutex);
    const AVTUK_CCU::Indication indication = transform(code);
    qDebug() << "Indication is: cnt1: " << indication.PulseCnt1 << ", freq1: " << indication.PulseFreq1
             << ", cnt2: " << indication.PulseCnt2 << ", freq2: " << indication.PulseFreq2;
    DataTypes::BlockStruct block;
    block.ID = AVTUK_CCU::IndicationBlock;
    block.data.resize(sizeof(indication));
    qDebug() << "Sizeof indication block: " << sizeof(indication);
    memcpy(block.data.data(), &indication, sizeof(indication));
    qDebug() << block;
    m_interface->writeCommand(Interface::Commands::C_WriteUserValues, QVariant::fromValue(block));
#endif
}

bool StmBroker::status()
{
    return m_status;
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

AVTUK_CCU::Indication StmBroker::transform(alise::Health_Code code) const
{
    constexpr auto maxFreq = 4000;
    constexpr auto minFreq = 1000;
    switch (code)
    {
    case alise::Health_Code_Startup:
    {
        return { 1, maxFreq, 0, 0 };
    }
    case alise::Health_Code_Work:
    {
        return { 1, minFreq, 0, 0 };
    }
    case alise::Health_Code_Update:
    {
        return { 1, minFreq, static_cast<uint8_t>(code), maxFreq };
    }
    case alise::Health_Code_StartupFail:
    {
        return { 1, minFreq, static_cast<uint8_t>(code), maxFreq };
    }
    case alise::Health_Code_NoProject:
    {
        return { 1, minFreq, static_cast<uint8_t>(code), maxFreq };
    }
    case alise::Health_Code_ProjectError:
    {
        return { 1, minFreq, static_cast<uint8_t>(code), maxFreq };
    }
    case alise::Health_Code_NoFirmware:
    {
        return { 1, minFreq, static_cast<uint8_t>(code), maxFreq };
    }
    case alise::Health_Code_BootloaderError:
    {
        return { 1, minFreq, static_cast<uint8_t>(code), maxFreq };
    }
    case alise::Health_Code_SettingsError:
    {
        return { 1, minFreq, static_cast<uint8_t>(code), maxFreq };
    }
    default:
        return transform(alise::Health_Code_SettingsError);
    }
}

timespec StmBroker::transform(google::protobuf::Timestamp timestamp) const
{
    timespec temp;
    temp.tv_nsec = timestamp.nanos();
    temp.tv_sec = timestamp.seconds();
    return temp;
}
