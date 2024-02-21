#pragma once

#include <interfaces/exec/default_query_executor.h>
#include <interfaces/types/settingstypes.h>

namespace Interface
{

/// \brief Фабрика для создания экземпляров класса DeviceQueryExecutor.
class QueryExecutorFabric
{
public:
    /// \brief Удалённый конструктор по умолчанию.
    /// \details Not constructible type.
    QueryExecutorFabric() = delete;
    /// \brief Удалённый деструктор.
    /// \details Not constructible type.
    ~QueryExecutorFabric() = delete;

    /// \brief Создание исполнителя запросов к устройству по протоколу Protocom.
    static DefaultQueryExecutor *makeProtocomExecutor(RequestQueue &queue, const UsbHidSettings &settings);
};

} // namespace Interface
