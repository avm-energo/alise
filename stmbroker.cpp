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

#ifdef TEST_INDICATOR
    m_testTimer.setInterval(10000);
    QObject::connect(&m_testTimer, &QTimer::timeout, this,
        [this] { setIndication(static_cast<alise::Health_Code>(QRandomGenerator::global()->bounded(0, 8))); });
    m_testTimer.start();
#endif
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

void StmBroker::setIndication()
{
#ifndef ALISE_LOCALDEBUG
    QMutexLocker locker(&_mutex);
    const AVTUK_CCU::Indication indication = transformBlinkPeriod();
    qDebug() << "Indication is: cnt1: " << indication.PulseCnt1 << ", freq1: " << indication.PulseFreq1
             << ", cnt2: " << indication.PulseCnt2 << ", freq2: " << indication.PulseFreq2;
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

AVTUK_CCU::Indication StmBroker::transformBlinkPeriod() const
{
    quint16 blinkfreq = 1000 / m_currentBlinkingPeriod * 1000; // transform to mHz from ms
    return { 1, blinkfreq, 0, 0 };
}

timespec StmBroker::transform(google::protobuf::Timestamp timestamp) const
{
    timespec temp;
    temp.tv_nsec = timestamp.nanos();
    temp.tv_sec = timestamp.seconds();
    return temp;
}
