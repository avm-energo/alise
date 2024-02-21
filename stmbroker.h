#pragma once

#include "avtukccu.h"
#include "broker.h"
#include "protos/protos.pb.h"

#include <QMutex>
#include <QObject>
#include <QTimer>
#include <gen/datatypes.h>
#include <gen/stdfunc.h>

namespace Interface
{
class AsyncConnection;
class ConnectionManager;
} // namespace Interface

class StmBroker final : public Broker
{
public:
    explicit StmBroker(QObject *parent = nullptr);
    bool connect() override;

public slots:
    void checkPowerUnit() override;
    void checkIndication() override;
    void setIndication(const AVTUK_CCU::Indication &indication) override;
    void setTime(timespec time) override;
    void getTime() override;
    void rebootMyself() override;
    void currentIndicationReceived(const DataTypes::BlockStruct &blk);

private:
    timespec transform(google::protobuf::Timestamp timestamp) const;
    QMutex _mutex;
    Interface::ConnectionManager *m_manager;
    Interface::AsyncConnection *m_conn;

#ifdef TEST_INDICATOR
    QTimer m_testTimer;
#endif
};
