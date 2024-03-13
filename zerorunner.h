#pragma once
#include "gen/stdfunc.h"
#include "protos/protos.pb.h"
#include "zeropublisher.h"
#include "zerosubscriber.h"

#include <QMutex>
#include <QObject>
#include <zmq.hpp>

constexpr uint32_t timeout = 300000;

class ZeroRunner : public QObject
{
    Q_OBJECT
public:
    ZeroRunner(const QString &type, QObject *parent = nullptr);

public slots:
    void runServer(int port);
    void stopServer();

    void publishHealthQueryCallback();
    void publishNtpStatus(bool);
    void publishBlock(const DataTypes::BlockStruct &blk);
    void publishTime(const timespec &time);

signals:
    void timeRequest();

    void healthReceived(uint32_t);
    void timeReceived(timespec);

private:
    void proxy(zmq::socket_t &front, zmq::socket_t &sub, zmq::socket_t &pub);
    void polling();

    QMutex _mutex;

    zmq::context_t ctx_;
    zmq::socket_t frontend_;
    zmq::socket_t backendSub_;
    zmq::socket_t backendPub_;

    //    UniquePointer<DataTypesProxy> proxyBS, proxyTS;
    ZeroSubscriber *m_subscriber;
    ZeroPublisher *m_publisher;
};
