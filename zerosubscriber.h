#pragma once
#include "protos/protos.pb.h"

#include <QMutex>
#include <QObject>
#include <QWaitCondition>
#include <vector>
#include <zmq.hpp>

class ZeroSubscriber : public QObject
{
    Q_OBJECT
public:
    explicit ZeroSubscriber(zmq::context_t &ctx, int sock_type, QObject *parent = nullptr);
    void work();
    void stop()
    {
        is_active = false;
    }
signals:
    void timeReceived(timespec);
    void timeRequest();
    void pingRequest();
    void healthReceived(uint32_t);
    void helloReceived();

private:
    zmq::context_t &_ctx;
    zmq::socket_t _worker;
    QMutex _mutex;
    QWaitCondition _waiter;
    bool is_active = true;
};
