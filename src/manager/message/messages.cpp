#include "messages.hpp"

#include <boost/uuid/uuid_io.hpp>

namespace ch {
namespace manager {

nlohmann::json worker_manager_crackHashSubRequestReady(
    boost::uuids::uuid crackHashRequestUuid,
    boost::uuids::uuid crackHashSubRequestUuid,
    const std::vector<std::string> &result
) {
    return {
        {"request_id", boost::uuids::to_string(crackHashRequestUuid)},
        {"sub_request_id", boost::uuids::to_string(crackHashSubRequestUuid)},
        {"result", result}
    };
}

} // namespace manager
} // namespace ch