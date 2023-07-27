#include "zerorunner.h"

#include "gen/stdfunc.h"
#include "timesyncronizer.h"

#include <QDebug>
#include <QMutexLocker>
#include <QTimer>
#include <fstream>
#include <gen/datatypes.h>
#include <iostream>
#include <thread>

namespace runner
{

ZeroRunner::ZeroRunner(QObject *parent)
    : QObject(parent)
    , ctx_(1) // 1 = io_threads
    , frontend_(ctx_, ZMQ_ROUTER)
    , backendSub_(ctx_, ZMQ_DEALER)
    , backendPub_(ctx_, ZMQ_DEALER)
    , proxyBS(new DataTypesProxy)
    , proxyTS(new DataTypesProxy)
{
    proxyBS->RegisterType<DataTypes::BlockStruct>();
    proxyTS->RegisterType<timespec>();
}

void ZeroRunner::runServer(int port)
{
    qRegisterMetaType<ZeroSubscriber::healthType>("healthType");
    qRegisterMetaType<timespec>();
    qRegisterMetaType<AVTUK_CCU::Main>();
    frontend_.bind("tcp://*:" + std::to_string(port));
    backendSub_.bind("inproc://backendSub");
    backendPub_.bind("inproc://backendPub");

    m_subscriber = UniquePointer<ZeroSubscriber>(new ZeroSubscriber(ctx_, ZMQ_DEALER));
    m_publisher = UniquePointer<ZeroPublisher>(new ZeroPublisher(ctx_, ZMQ_DEALER));

    connect(m_subscriber.get(), &ZeroSubscriber::helloReceived, m_publisher.get(), &ZeroPublisher::publishHello,
        Qt::DirectConnection);

    connect(m_subscriber.get(), &ZeroSubscriber::timeRequest, this, &ZeroRunner::timeRequest);
    connect(m_subscriber.get(), &ZeroSubscriber::timeReceived, this, &ZeroRunner::timeReceived);

    connect(m_subscriber.get(), &ZeroSubscriber::healthReceived, this, &ZeroRunner::healthReceived);

    auto subscriber = std::unique_ptr<std::thread>(new std::thread([&, worker = std::move(m_subscriber)] {
        //        connect(worker.get(), &ZeroSubscriber::timeRequest, this, &ZeroRunner::timeRequest);
        worker->work();
    }));

    //    proxyBS = UniquePointer<DataTypesProxy>(new DataTypesProxy());
    //    proxyBS->RegisterType<DataTypes::BlockStruct>();

    //#ifdef __linux__
    //    proxyTS = UniquePointer<DataTypesProxy>(new DataTypesProxy(&DataManager::GetInstance()));
    //    proxyTS->RegisterType<timespec>();
    //#endif

    auto publisher = std::unique_ptr<std::thread>(new std::thread([&, worker = std::move(m_publisher)] {
        //        connect(proxyTS.get(), &DataTypesProxy::DataStorable, worker.get(), &ZeroPublisher::publishTime,
        //            Qt::DirectConnection);
        //        connect(
        //            this, &ZeroRunner::publishNtpStatus, worker.get(), &ZeroPublisher::publishNtpStatus,
        //            Qt::DirectConnection);
        //        connect(proxyBS.get(), &DataTypesProxy::DataStorable, worker.get(), &ZeroPublisher::publishBlock,
        //            Qt::DirectConnection);
        worker->work();
    }));

    publisher->detach();
    subscriber->detach();

    auto controller = std::unique_ptr<std::thread>(new std::thread([&] { polling(); }));

    controller->detach();
    qInfo() << "ZeroRunner started";
}

void ZeroRunner::stopServer()
{
}

void ZeroRunner::publishHealthQueryCallback()
{
    m_publisher.get()->publishHealthQuery();
}

void ZeroRunner::publishNtpStatus(bool status)
{
    m_publisher.get()->publishNtpStatus(status);
}

void ZeroRunner::publishBlock(const QVariant &msg)
{
    auto blk = msg.value<DataTypes::BlockStruct>();
    m_publisher.get()->publishBlock(blk);
}

void ZeroRunner::publishTime(const QVariant &msg)
{
    m_publisher.get()->publishTime(msg);
}

void ZeroRunner::polling()
{
    //  proxy(frontend_, backendSub_, backendPub_);
    zmq_pollitem_t items[] = {
        { frontend_, 0, ZMQ_POLLIN, 0 },   //
        { backendPub_, 0, ZMQ_POLLIN, 0 }, //
        { backendSub_, 0, ZMQ_POLLIN, 0 }  //
    };
    try
    {
        while (1)
        {
            zmq::message_t message;
            int more; //  Multipart detection

            zmq::poll(&items[0], 2, -1);

            if (items[0].revents & ZMQ_POLLIN)
            {
                while (1)
                {
                    //  Process all parts of the message
                    [[maybe_unused]] auto len = frontend_.recv(message, zmq::recv_flags::none);
                    more = frontend_.get(zmq::sockopt::rcvmore);
                    backendSub_.send(message, more ? zmq::send_flags::sndmore : zmq::send_flags::none);

                    if (!more)
                        break; //  Last message part
                }
            }
            if (items[1].revents & ZMQ_POLLIN)
            {
                while (1)
                {
                    //  Process all parts of the message
                    [[maybe_unused]] auto len = backendPub_.recv(message);
                    more = backendPub_.get(zmq::sockopt::rcvmore);
                    frontend_.send(message, more ? zmq::send_flags::sndmore : zmq::send_flags::none);

                    if (!more)
                        break; //  Last message part
                }
            }
        }

    } catch (std::exception &e)
    {
        qDebug() << "Exception: " << e.what();
        return;
    }
    frontend_.close();
    backendPub_.close();
    backendSub_.close();
    ctx_.close();
}

}
