#include "messages.hpp"

#include "common/crack_hash_request/utilities.hpp"

#include <boost/uuid/uuid_io.hpp>

namespace ch {
namespace client {

nlohmann::json manager_client_addedCrackHashRequest(
    boost::uuids::uuid crackHashRequestUuid
) {
    return nlohmann::json {
        {"request_id", boost::uuids::to_string(crackHashRequestUuid)}
    };
}

nlohmann::json manager_client_crackHashRequestStatus(
    boost::uuids::uuid crackHashRequestUuid,
    common::CrackHashRequestStatus status,
    std::vector<std::string> data
) {
    return nlohmann::json {
        {"request_id", boost::uuids::to_string(crackHashRequestUuid)},
        {"status", common::crackHashRequestStatusToString(status)},
        {"data", data}
    };
}

} // namespace client
} // namespace ch