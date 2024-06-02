#pragma once

#include "common/crack_hash_request/CrackHashRequestStatus.hpp"

#include "tp/nlohmann/json.hpp"

#include <boost/uuid/uuid.hpp>

#include <memory>

namespace ch {
namespace client {

nlohmann::json manager_client_addedCrackHashRequest(
    boost::uuids::uuid crackHashRequestUuid);

nlohmann::json manager_client_crackHashRequestStatus(
    boost::uuids::uuid crackHashRequestUuid,
    common::CrackHashRequestStatus status,
    std::vector<std::string> data = {});

} // namespace client
} // namespace ch