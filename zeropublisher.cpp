#include "zeropublisher.h"

#include "protos.pb.h"

#include <QDebug>
#include <functional>
#include <memory>
#include <random>
#include <thread>

ZeroPublisher::ZeroPublisher(zmq::context_t &ctx, int sock_type, QObject *parent)
    : QObject(parent), _ctx(ctx), _worker(_ctx, sock_type)
{
}

void ZeroPublisher::work()
{
    try
    {
        _worker.connect("inproc://backendPub");
        while (is_active)
        {
            QMutexLocker locker(&_mutex);
            checkQueue();
            _waiter.wait(&_mutex);
        }
    } catch (std::exception &e)
    {
        qDebug() << "Exception: " << e.what();
    }
}

template <typename T> void ZeroPublisher::appendToQueue(const std::string &id, const T &paylod)
{
    alise::PackedMessage packedMessage;
    packedMessage.mutable_content()->PackFrom(paylod);
    std::string serialized_update;
    packedMessage.SerializeToString(&serialized_update);
    //  qDebug() << paylod.DebugString().c_str();
    _mutex.lock();
    _queue.push({ id, serialized_update });
    _mutex.unlock();
    _waiter.wakeOne();
}

void ZeroPublisher::publishTime(const timespec time)
{
    qDebug() << "Time has been added to output queue: " << time.tv_sec;
    google::protobuf::Timestamp protoTime;
    protoTime.set_seconds(time.tv_sec);
    protoTime.set_nanos(time.tv_nsec);
    appendToQueue(admin, protoTime);
}

void ZeroPublisher::publishPowerStatus(const AVTUK_CCU::Main powerStatus)
{
    qDebug() << "PowerStatus has been added to output queue: " << powerStatus.PWRIN << ", resetReq: " << powerStatus.resetReq;
    alise::PowerStatus protoPower;
    protoPower.set_pwrin(powerStatus.PWRIN);
    appendToQueue(sonica, protoPower);
}

void ZeroPublisher::publishBlock(const DataTypes::BlockStruct blk)
{
    switch (blk.data.size())
    {
    case sizeof(AVTUK_CCU::Main):
    {
        AVTUK_CCU::Main powerStatus;
        memcpy(&powerStatus, blk.data.data(), sizeof(powerStatus));
        publishPowerStatus(powerStatus);
    }
    }
}

void ZeroPublisher::publishHello(const QString id, const quint32 code)
{
    qDebug() << "HelloReply has been added to output queue: " << id << ", code: " << code;
    alise::HelloReply helloReply;
    helloReply.set_message(code + 1);
    appendToQueue(id.toStdString(), helloReply);
}

void ZeroPublisher::publishNtpStatus(bool status)
{
    qDebug() << "NtpStatus has been added to output queue: " << status;
    alise::NtpStatus ntpStatus;
    ntpStatus.set_isntpenabled(status);
    appendToQueue(admin, ntpStatus);
}

void ZeroPublisher::publishHealthQuery()
{
    qDebug() << "HealthQuery has been added to output queue";
    alise::HealthQuery query;
    query.set_query(true);
    appendToQueue(sonica, query);
}

void ZeroPublisher::send(itemType &str)
{
    zmq::message_t identity(str.first);
    zmq::message_t msg(str.second);
    qInfo() << "Send message to: {" << str.first.c_str() << "}, with payload: {" << str.second.c_str() << "}";
    _worker.send(identity, zmq::send_flags::sndmore);
    _worker.send(msg, zmq::send_flags::none);
}

void ZeroPublisher::checkQueue()
{
    while (!_queue.empty())
    {
        send(_queue.back());
        _queue.pop();
    }
}
