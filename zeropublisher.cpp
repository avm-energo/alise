#include "zeropublisher.h"

#include "protos/protos.pb.h"

#include <QDebug>
#include <config.h>
#include <functional>
#include <memory>
#include <random>
#include <thread>

ZeroPublisher::ZeroPublisher(const QString &type, zmq::context_t &ctx, int sock_type, QObject *parent)
    : QObject(parent), _ctx(ctx), _worker(_ctx, sock_type)
{
    m_type = "[ " + type + " ]";
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
        qDebug() << "Exited from worker loop";
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

void ZeroPublisher::publishTime(const QVariant &msg)
{
    auto time = msg.value<timespec>();
    google::protobuf::Timestamp protoTime;
    protoTime.set_seconds(time.tv_sec);
    protoTime.set_nanos(time.tv_nsec);
    appendToQueue(sonicacore, protoTime);
    qDebug() << m_type + "Time => Q : " << time.tv_sec;
}

void ZeroPublisher::publishPowerStatus(const AVTUK_CCU::Main powerStatus)
{
    alise::PowerStatus protoPower;
    protoPower.set_pwrin(powerStatus.PWRIN);
    appendToQueue(sonicacore, protoPower);
    qDebug() << m_type + "Power => Q : " << powerStatus.PWRIN;
    qDebug() << m_type + "Reset => Q : " << powerStatus.resetReq;
}

void ZeroPublisher::publishBlock(const DataTypes::BlockStruct blk)
{
    if (blk.ID == AVTUK_CCU::MainBlock)
    {
        AVTUK_CCU::Main powerStatus;
        memcpy(&powerStatus, blk.data.data(), sizeof(powerStatus));
        publishPowerStatus(powerStatus);
    }
}

void ZeroPublisher::publishHello(const QString id, const quint32 code)
{
    alise::HelloReply helloReply;
    helloReply.set_message(COMAVERSION);
    appendToQueue(id.toStdString(), helloReply);
    qDebug() << m_type + "HelloReply => Q : " << id << ", code: " << code;
}

void ZeroPublisher::publishNtpStatus(bool status)
{
    alise::NtpStatus ntpStatus;
    ntpStatus.set_isntpenabled(status);
    appendToQueue(sonicacore, ntpStatus);
    qDebug() << m_type + "Ntp => Q : " << status;
}

void ZeroPublisher::publishHealthQuery()
{
    alise::HealthQuery query;
    query.set_query(true);
    appendToQueue(booter, query);
    qDebug() << m_type + "Health => Q";
}

void ZeroPublisher::send(itemType &str)
{
    zmq::message_t identity(str.first);
    zmq::message_t msg(str.second);
    //    qDebug() << "Send message to: {" << str.first.c_str() << "}, with payload: {" << str.second.c_str() << "}";
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
