#pragma once

#include <QThread>
#include <QThreadPool>
#include <interfaces/ifaces/baseinterface.h>

namespace Interface
{

class AsyncConnection;
class DefaultQueryExecutor;

/// \brief Класс для представления контекста выполнения интерфейса
/// устройства и исполнителя запросов к устройству в рамках текущего соединения.
class ConnectionContext
{
    friend class ConnectionManager;

private:
    BaseInterface *m_iface;
    DefaultQueryExecutor *m_executor;
    std::pair<QThread *, QThread *> m_syncThreads;

public:
    explicit ConnectionContext() noexcept;

    /// \brief Фунция для проверки, содержит ли контекст соединения валидные данные.
    bool isValid() const noexcept;

    /// \brief Инициализация контекста соединения с устройством.
    /// \param connPolicy [in] - для взаимодействия интерфейса и парсера
    /// по USB и последовательному порту используются разные типы коммуникации.
    /// Причина: использование QSerialPort, с которым нельзя работать из разных потоков.
    void init(BaseInterface *iface, DefaultQueryExecutor *executor, const Qt::ConnectionType connPolicy);
    /// \brief Запускает соединение для текущего инициализированного контекста.
    bool run(AsyncConnection *connection);
    /// \brief Сбрасывает текущий контекст и закрывает активное соединение.
    void reset();
};

} // namespace Interface
