#pragma once

#include "httpmiddleware.h"

#include <QHttpServer>
#include <QObject>
#include <QTcpServer>
#include <QTimer>

/*!
    \brief Класс для реализации обмена http-запросами между Соникой и Алисой

    Класс предназначен для реализации обмена между Соникой и Алисой
    с помощью REST-запросов GET и POST с телом в виде JSON-объектов.
    Работа с классом производится следующим образом:

    \code
        qint16 port = 5345;
        HttpApiServer server;
        server.setRoutes();
        if (!server.initServer(port))
            qDebug() << "Failed to open http server!";
    \endcode
*/

class HttpApiServer : public QObject
{
    Q_OBJECT
public:
    explicit HttpApiServer(HttpMiddleware *mw, QObject *parent = nullptr);

    /// \brief Инициализация сервера
    /// \details Инициализация и старт сервера HttpApiServer
    /// \param[in] port Порт, на котором работает HttpApiServer, адрес берётся QHttp::Any
    /// \return Состояние сервера: true - открыт, false - ошибка при открытии
    bool initServer(int port, int pingTimeout);

signals:

private:
    HttpMiddleware *m_mw; ///< экземпляр MiddleWare - прослойка между основной Алисой и HttpApiServer
    QTcpServer *m_tcpServer;
    QHttpServer m_server;       ///< экземпляр http-сервера
    QTimer *m_pingTimeoutTimer; ///< таймер для отслеживания неприходящих запросов Ping
    int m_pingTimeout;          ///< число в мс, отражающее таймаут команд Ping

    /// \brief Установка маршрутов и слотов
    /// \details Настройка сопоставлений между http-запросами и конкретными слотами
    void setRoutes();

private slots:
    void pingTimeoutTimerTimeout();
};
