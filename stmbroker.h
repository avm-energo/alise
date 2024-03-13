#pragma once

#include "alisesettings.h"
#include "avtukccu.h"
#include "broker.h"

#include <QMutex>
#include <QObject>
#include <QTimer>
#include <gen/stdfunc.h>

namespace Interface
{
class AsyncConnection;
class ConnectionManager;
} // namespace Interface

class StmBroker final : public Broker
{
    Q_OBJECT
    static constexpr int HWAdr = 3;
    static constexpr int SWAdr = 4;
    static constexpr int SerialNumBAdr = 10;
    static constexpr int ModuleSerialNumAdr = 13;

public:
    explicit StmBroker(QObject *parent = nullptr);
    bool connect() override;
    bool connect(AliseSettings &asettings);
    void writeHiddenBlock();

public slots:
    void checkPowerUnit() override;
    void checkIndication() override;
    void setIndication(const AVTUK_CCU::Indication &indication) override;
    void setTime(const timespec &time) override;
    void getTime() override;
    void rebootMyself() override;
    void currentIndicationReceived(const DataTypes::BlockStruct &blk);

private:
    timespec transform(google::protobuf::Timestamp timestamp) const;
    QMutex _mutex;
    Interface::ConnectionManager *m_manager;
    Interface::AsyncConnection *m_conn;
    //    AliseSettings m_settings;

private slots:
    void updateBsi(AliseSettings &m_settings, const DataTypes::BitStringStruct &resp);

signals:
    void ModuleInfoFilled();
};
