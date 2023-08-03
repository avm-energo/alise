#pragma once

#include "avtukccu.h"
#include "broker.h"
#include "protos/protos.pb.h"

#include <QObject>
#include <QTimer>
#include <gen/datamanager/typesproxy.h>
#include <gen/datatypes.h>
#include <gen/stdfunc.h>

//#define TEST_INDICATOR

class Protocom;

class StmBroker : public Broker
{
public:
    StmBroker(QObject *parent = nullptr);
    bool connect() override;

public slots:
    void checkPowerUnit() override;
    void setIndication() override;
    void setTime(timespec time) override;
    void getTime() override;
    void rebootMyself() override;
    void currentIndicationReceived(const QVariant &msg) override;

private:
    AVTUK_CCU::Indication transformBlinkPeriod() const;
    timespec transform(google::protobuf::Timestamp timestamp) const;
    QMutex _mutex;
    Protocom *m_interface;
    //    UniquePointer<DataTypesProxy> proxyBS, proxyBStr;

#ifdef TEST_INDICATOR
    QTimer m_testTimer;
#endif
};
