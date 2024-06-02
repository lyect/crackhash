#include "messages.hpp"

#include <boost/uuid/uuid_io.hpp>

namespace ch {
namespace worker {

nlohmann::json manager_worker_addCrackHashSubRequest(
    boost::uuids::uuid crackHashRequestUuid,
    boost::uuids::uuid crackHashSubRequestUuid,
    std::shared_ptr<common::CrackHashRequest> crackHashSubRequest
) {
    return {
        {"request_id",     boost::uuids::to_string(crackHashRequestUuid)},
        {"sub_request_id", boost::uuids::to_string(crackHashSubRequestUuid)},
        {"hash",           crackHashSubRequest->hash()},
        {"max_length",     crackHashSubRequest->maxLength()},
        {"alphabet",       crackHashSubRequest->alphabet()},
        {"from",           crackHashSubRequest->from()},
        {"to",             crackHashSubRequest->to()}
    };
}

} // namespace worker
} // namespace ch