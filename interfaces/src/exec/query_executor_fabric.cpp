#include "interfaces/exec/query_executor_fabric.h"

#include <interfaces/parsers/protocom_request_parser.h>
#include <interfaces/parsers/protocom_response_parser.h>

namespace Interface
{

DefaultQueryExecutor *QueryExecutorFabric::makeProtocomExecutor(RequestQueue &queue, const UsbHidSettings &settings)
{
    auto executor = new DefaultQueryExecutor(queue, settings);
    executor->initLogger("Protocom");
    // NOTE: query executor must be parent for all parsers
    auto requestParser = new ProtocomRequestParser(executor);
    auto responseParser = new ProtocomResponseParser(executor);
    executor->setParsers(requestParser, responseParser);
    return executor;
}

} // namespace Interface
