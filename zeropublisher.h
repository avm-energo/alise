#pragma once
#include "avtukccu.h"

#include <QMutex>
#include <QObject>
#include <QWaitCondition>
#include <gen/datatypes.h>
#include <queue>
#include <vector>
#include <zmq.hpp>
Q_DECLARE_METATYPE(AVTUK_CCU::Main);
Q_DECLARE_METATYPE(AVTUK_CCU::Indication);

#define LOG_PROTOBUF

class ZeroPublisher : public QObject
{
    Q_OBJECT
public:
    using itemType = std::pair<std::string, std::string>;
    using queueType = std::queue<itemType>;

    explicit ZeroPublisher(const QString &type, zmq::context_t &ctx, int sock_type, QObject *parent = nullptr);

    void work();
    void stop()
    {
        is_active = false;
    }

signals:
public slots:
    void publishTime(const QVariant &msg);
    void publishPowerStatus(const AVTUK_CCU::Main powerStatus);
    void publishBlock(const DataTypes::BlockStruct blk);
    void publishHello(const quint32 code);
    void publishNtpStatus(bool status);
    void publishHealthQuery();

private:
    zmq::context_t &_ctx;
    zmq::socket_t _worker;
    QMutex _mutex;
    QWaitCondition _waiter;
    bool is_active = true;
    queueType _queue;
    QString m_typeString;
    std::string m_type;

    void send(itemType &str);
    void checkQueue();

    template <typename T> void appendToQueue(const T &paylod);
};
