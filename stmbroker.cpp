#include "stmbroker.h"

#include "../gen/helper.h"
#include "../gen/stdfunc.h"
#include "../interfaces/baseinterface.h"
#include "../interfaces/protocom.h"
#include "../interfaces/usbhidportinfo.h"
#include "../module/board.h"
#include "controller.h"
#include "helper.h"
#include "timesyncronizer.h"

#include <QRandomGenerator>

StmBroker::StmBroker(QObject *parent) : QObject(parent), proxyBS(new DataTypesProxy), proxyBStr(new DataTypesProxy)
{
    proxyBS->RegisterType<DataTypes::BlockStruct>();
    proxyBStr->RegisterType<DataTypes::BitStringStruct>();
}

bool StmBroker::connectToStm()
{
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
    BaseInterface::InterfacePointer device;
    device = BaseInterface::InterfacePointer(new Protocom());
    BaseInterface::setIface(std::move(device));

    m_interface = static_cast<Protocom *>(BaseInterface::iface());
    if (!m_interface->start(devices.first()))
    {
        std::cout << "Couldn't connect" << std::endl;
        return false;
    }
    const auto &board = Board::GetInstance();
    QObject::connect(proxyBStr.get(), &DataTypesProxy::DataStorable, &board, &Board::update);
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
    return true;
}

void StmBroker::checkPowerUnit()
{
    Q_CHECK_PTR(m_interface);
    m_interface->writeCommand(Queries::QUSB_ReqBlkData, AVTUK_CCU::MainBlock);
}

void StmBroker::setIndication(alise::Health_Code code)
{
    const auto indication = transform(code);
    DataTypes::BlockStruct block;
    block.ID = AVTUK_CCU::IndicationBlock;
    block.data.resize(sizeof(decltype(indication)));
    memcpy(block.data.data(), &indication, sizeof(indication));
    qDebug() << block;
    m_interface->writeCommand(Queries::QC_WriteUserValues, QVariant::fromValue(block));
}

void StmBroker::setTime(timespec time)
{
    Q_CHECK_PTR(m_interface);
    m_interface->writeTime(time);
}

void StmBroker::getTime()
{
    Q_CHECK_PTR(m_interface);
    m_interface->reqTime();
}

void StmBroker::rebootMyself()
{
    m_interface->writeCommand(Queries::QUSB_Reboot, 0xff);
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

void StmBroker::printbs(const DataTypes::BitStringStruct &st)
{
    std::cout << "BitString {"
              << "Addr:" << st.sigAdr << ","
              << "Val:" << st.sigVal << ","
              << "Qual:" << st.sigQuality << " }" << std::endl;
}
