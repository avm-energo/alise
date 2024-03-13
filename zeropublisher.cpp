#include "zeropublisher.h"

#include "alisesettings.h"
#include "gitversion/gitversion.h"
#include "protos/protos.pb.h"

#include <QDebug>
#include <config.h>
#include <functional>
#include <gen/datatypes.h>
#include <memory>
#include <random>
#include <thread>

ZeroPublisher::ZeroPublisher(const QString &type, zmq::context_t &ctx, int sock_type, QObject *parent)
    : QObject(parent), _ctx(ctx), _worker(_ctx, sock_type)
{
    m_typeString = "[ " + type + "out ]";
    m_type = type.toStdString();
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

template <typename T> void ZeroPublisher::appendToQueue(const T &paylod)
{
    alise::PackedMessage packedMessage;
    packedMessage.mutable_content()->PackFrom(paylod);
    std::string serialized_update;
    packedMessage.SerializeToString(&serialized_update);
    _mutex.lock();
    _queue.push({ m_type, serialized_update });
    _mutex.unlock();
    _waiter.wakeOne();
}

void ZeroPublisher::publishTime(const timespec &time)
{
    google::protobuf::Timestamp protoTime;
    protoTime.set_seconds(time.tv_sec);
    protoTime.set_nanos(time.tv_nsec);
    appendToQueue(protoTime);
    qDebug() << m_typeString + " Time : " << time.tv_sec;
}

void ZeroPublisher::publishPowerStatus(const AVTUK_CCU::Main powerStatus)
{
    alise::PowerStatus protoPower;
    protoPower.set_pwrin(powerStatus.PWRIN);
    protoPower.set_resetreq(powerStatus.resetReq);
    appendToQueue(protoPower);
    qDebug() << m_typeString + " Power : " << powerStatus.PWRIN;
    qDebug() << m_typeString + " Reset : " << powerStatus.resetReq;
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

void ZeroPublisher::publishHello()
{
    AliseSettings settings;
    settings.init();
    settings.readSettings();
    alise::HelloReply helloReply;
    GitVersion gitVersion;
    QString AliseVersion = QString(ALISEVERSION) + "-" + gitVersion.getGitHash();
    helloReply.set_message(AliseVersion.toStdString());
    helloReply.set_hwversion(settings.versionStr(settings.hwVersion).toStdString());
    helloReply.set_serialnum(QString::number(settings.serialNum).toStdString());
    helloReply.set_swversion(settings.versionStr(settings.swVersion).toStdString());
    appendToQueue(helloReply);
    qDebug() << m_typeString + " HelloReply version: " << ALISEVERSION;
}

void ZeroPublisher::publishNtpStatus(bool status)
{
    alise::NtpStatus ntpStatus;
    ntpStatus.set_isntpenabled(status);
    appendToQueue(ntpStatus);
    qDebug() << m_typeString + " Ntp : " << status;
}

void ZeroPublisher::publishHealthQuery()
{
    alise::HealthQuery query;
    query.set_query(true);
    appendToQueue(query);
    qDebug() << m_typeString + " Health";
}

void ZeroPublisher::send(itemType &str)
{
    zmq::message_t identity(str.first);
    zmq::message_t msg(str.second);
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
