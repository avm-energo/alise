#pragma once
#include "../gen/datatypes.h"
#include "avtukccu.h"

#include <QMutex>
#include <QObject>
#include <QWaitCondition>
#include <queue>
#include <vector>
#include <zmq.hpp>
Q_DECLARE_METATYPE(AVTUK_CCU::Main);
Q_DECLARE_METATYPE(AVTUK_CCU::Indication);

#define LOG_PROTOBUF

constexpr char booter[] = "sb";      // booter
constexpr char sonicablock[] = "sc"; // core block
constexpr char sonicacore[] = "sa";  // core

// template <typename T> void appendToQueue(std::string &&id, const T &paylod);

class ZeroPublisher : public QObject
{
    //   template <typename T> friend void appendToQueue(std::string &&id, const T &paylod);

    Q_OBJECT
public:
    using itemType = std::pair<std::string, std::string>;
    using queueType = std::queue<itemType>;

    explicit ZeroPublisher(zmq::context_t &ctx, int sock_type, QObject *parent = nullptr);

    void work();
    void stop()
    {
        is_active = false;
    }

signals:
public slots:
    void publishTime(const QVariant &msg);
    void publishPowerStatus(const AVTUK_CCU::Main powerStatus);
    void publishBlock(const QVariant &msg);
    void publishHello(const QString id, const quint32 code);
    void publishNtpStatus(bool status);
    void publishHealthQuery();

private:
    zmq::context_t &_ctx;
    zmq::socket_t _worker;
    QMutex _mutex;
    QWaitCondition _waiter;
    bool is_active = true;
    queueType _queue;

    void send(itemType &str);
    void checkQueue();

    template <typename T> void appendToQueue(const std::string &id, const T &paylod);
};
