#include "zerorunner.h"

#include "gen/stdfunc.h"
#include "timesyncronizer.h"

#include <QDebug>
#include <QMutexLocker>
#include <QTimer>
#include <fstream>
#include <iostream>
#include <thread>

ZeroRunner::ZeroRunner(QObject *parent)
    : QObject(parent)
    , ctx_(1) // 1 = io_threads
    , frontend_(ctx_, ZMQ_ROUTER)
    , backendSub_(ctx_, ZMQ_DEALER)
    , backendPub_(ctx_, ZMQ_DEALER)
//    , proxyBS(new DataTypesProxy)
//    , proxyTS(new DataTypesProxy)
{
    //    proxyBS->RegisterType<DataTypes::BlockStruct>();
    //    proxyTS->RegisterType<timespec>();
    m_subscriber = new ZeroSubscriber(ctx_, ZMQ_DEALER);
    m_publisher = new ZeroPublisher(ctx_, ZMQ_DEALER);
}

void ZeroRunner::runServer(int port)
{
    qRegisterMetaType<ZeroSubscriber::healthType>("healthType");
    qRegisterMetaType<timespec>();
    qRegisterMetaType<AVTUK_CCU::Main>();
    frontend_.bind("tcp://*:" + std::to_string(port));
    backendSub_.bind("inproc://backendSub");
    backendPub_.bind("inproc://backendPub");

    connect(
        m_subscriber, &ZeroSubscriber::helloReceived, m_publisher, &ZeroPublisher::publishHello, Qt::DirectConnection);

    connect(m_subscriber, &ZeroSubscriber::timeRequest, this, &ZeroRunner::timeRequest);
    connect(m_subscriber, &ZeroSubscriber::timeReceived, this, &ZeroRunner::timeReceived);

    connect(m_subscriber, &ZeroSubscriber::healthReceived, this, &ZeroRunner::healthReceived);

    auto subscriber = std::unique_ptr<std::thread>(new std::thread([&] {
        //        connect(worker, &ZeroSubscriber::timeRequest, this, &ZeroRunner::timeRequest);
        m_subscriber->work();
    }));

    //    proxyBS = UniquePointer<DataTypesProxy>(new DataTypesProxy());
    //    proxyBS->RegisterType<DataTypes::BlockStruct>();

    //#ifdef __linux__
    //    proxyTS = UniquePointer<DataTypesProxy>(new DataTypesProxy(&DataManager::GetInstance()));
    //    proxyTS->RegisterType<timespec>();
    //#endif

    auto publisher = std::unique_ptr<std::thread>(new std::thread([&] {
        //        connect(proxyTS, &DataTypesProxy::DataStorable, worker, &ZeroPublisher::publishTime,
        //            Qt::DirectConnection);
        //        connect(
        //            this, &ZeroRunner::publishNtpStatus, worker, &ZeroPublisher::publishNtpStatus,
        //            Qt::DirectConnection);
        //        connect(proxyBS, &DataTypesProxy::DataStorable, worker, &ZeroPublisher::publishBlock,
        //            Qt::DirectConnection);
        m_publisher->work();
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
    m_publisher->publishHealthQuery();
}

void ZeroRunner::publishNtpStatus(bool status)
{
    m_publisher->publishNtpStatus(status);
}

void ZeroRunner::publishBlock(const QVariant &msg)
{
    auto blk = msg.value<DataTypes::BlockStruct>();
    m_publisher->publishBlock(blk);
}

void ZeroRunner::publishTime(const QVariant &msg)
{
    m_publisher->publishTime(msg);
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
