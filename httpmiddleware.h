#pragma once

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <gen/datatypes.h>

class HttpMiddleware : public QObject
{
    Q_OBJECT
public:
    explicit HttpMiddleware(QObject *parent = nullptr);

    QJsonObject helloReply();  ///< подготовка ответа на запрос Hello
    QJsonObject timeStamp();   ///< подготовка json с текущим временем
    QJsonObject pingReply();   ///< подготовка ответа на ping запрос
    QJsonObject powerStatus(); ///< подготовка json с текущим значеним состояния GPIO
    QJsonObject ntpStatus();   ///< подготовка json с текущим значеним NTP статуса
    void timePost(const QByteArray &arr);   ///< получили POST сообщение с временем
    void healthPost(const QByteArray &arr); ///< получили POST сообщение со статусами
    void setBooterConnectionLost(); ///< выдача сигнала о потере связи с Бутером

signals:
    void booterConnectionIsLost(); ///< сигнал при потере пингов от Бутера на время более двух запросов подряд
    void healthReceived(uint32_t code); ///< сигнал при получении состояния Соники от Бутера
    void timeReceived(
        const timespec &systemTime); ///< сигнал при получении текущего времени для задания в Алису из Соники

public slots:
    void setTimeStamp(const timespec &systemTime); ///< установка текущего времени по сигналу из брокера
    void setNtpState(bool state); ///< установка состояния сервера времени (false - не работает, true - работает) по
                                  ///< сигналу из timeSynchronizer
    void setBlock(const DataTypes::BlockStruct &blk);

private:
    timespec m_time;
    bool m_ntpState;
    int m_pwrIn, m_resetReq;

    /// \brief Конвертация POST-запроса в JSON объект
    QJsonObject byteArrayToJsonObject(const QByteArray &arr);
};
