#include "zerosubscriber.h"

#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <gen/error.h>

ZeroSubscriber::ZeroSubscriber(zmq::context_t &ctx, int sock_type, QObject *parent)
    : QObject(parent), _ctx(ctx), _worker(_ctx, sock_type)
{
    qRegisterMetaType<ZeroSubscriber::healthType>();
}

void ZeroSubscriber::work()
{
    try
    {
        _worker.connect("inproc://backendSub");
        while (is_active)
        {
            zmq::message_t identity;
            zmq::message_t msg;

            auto id = _worker.recv(identity);
            auto ms = _worker.recv(msg);
            std::string data(msg.to_string());
            std::string iden(identity.to_string());
            alise::PackedMessage packedMessage;
            packedMessage.ParseFromString(data);
            const auto &messageContent = packedMessage.content();
            if (messageContent.Is<alise::Health>())
            {
                alise::Health *protoHealth = new alise::Health;
                if (!messageContent.UnpackTo(protoHealth))
                {
                    qWarning() << Error::WriteError;
                    continue;
                }
                qDebug() << "[" << iden.c_str() << "] <= Health :" << protoHealth->code();
                emit healthReceived(protoHealth->code());
                delete protoHealth;
            }
            else if (messageContent.Is<google::protobuf::Timestamp>())
            {
                google::protobuf::Timestamp protoTime;
                if (!messageContent.UnpackTo(&protoTime))
                {
                    qWarning() << Error::WriteError;
                    continue;
                }
                timespec unixTime;
                unixTime.tv_sec = protoTime.seconds();
                unixTime.tv_nsec = protoTime.nanos();
                qDebug() << "[" << iden.c_str() << "] <= Time :" << unixTime.tv_sec << ":" << unixTime.tv_nsec;
                emit timeReceived(unixTime);
            }
            else if (messageContent.Is<alise::HelloRequest>())
            {
                alise::HelloRequest helloAlise;
                if (!messageContent.UnpackTo(&helloAlise))
                {
                    qWarning() << Error::WriteError;
                    continue;
                }
                qDebug() << "[" << iden.c_str() << "] <= HelloReq :" << helloAlise.message();
                emit helloReceived(helloAlise.message());
            }
            else if (messageContent.Is<alise::TimeRequest>())
            {
                alise::TimeRequest payload;
                if (!messageContent.UnpackTo(&payload))
                {
                    qWarning() << Error::WriteError;
                    continue;
                }
                qDebug() << "[" << iden.c_str() << "] <= TimeReq";
                emit timeRequest();
            }
            else
            {
                qDebug() << "Received id bytes: " << id.value();
                qDebug() << "Received id: " << identity.to_string().c_str();
                qDebug() << "Received msg bytes: " << ms.value();
                qDebug() << "Received msg: " << data.c_str();
                qCritical() << Error::WrongType;
            }
            QCoreApplication::processEvents();
        }
        _worker.close();
    } catch (std::exception &e)
    {
        qCritical() << "Exception: " << e.what();
    }
}
