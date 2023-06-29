#pragma once

#include "avtukccu.h"
#include "protos/protos.pb.h"

#include <QObject>
#include <QTimer>
#include <gen/datamanager/typesproxy.h>
#include <gen/datatypes.h>
#include <gen/stdfunc.h>

//#define TEST_INDICATOR

class Protocom;

class StmBroker : public QObject
{
    //    Q_OBJECT
public:
    StmBroker(QObject *parent = nullptr);
    bool connectToStm();

public /*slots*/:
    void checkPowerUnit();
    void setIndication(alise::Health_Code code);

    void setTime(timespec time);
    void getTime();
    void rebootMyself();

private:
    AVTUK_CCU::Indication transform(alise::Health_Code code) const;
    timespec transform(google::protobuf::Timestamp timestamp) const;
    QMutex _mutex;

    // Controller m_controller;
    Protocom *m_interface;
    QTimer m_timer;
    UniquePointer<DataTypesProxy> proxyBS, proxyBStr;

#ifdef TEST_INDICATOR
    QTimer m_testTimer;
#endif
};
