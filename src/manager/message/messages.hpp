#pragma once

#include "tp/nlohmann/json.hpp"

#include <boost/uuid/uuid.hpp>

namespace ch {
namespace manager {

nlohmann::json worker_manager_crackHashSubRequestReady(
    boost::uuids::uuid crackHashRequestUuid,
    boost::uuids::uuid crackHashSubRequestUuid,
    const std::vector<std::string> &result);

} // namespace manager
} // namespace ch