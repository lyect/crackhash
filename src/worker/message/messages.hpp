#pragma once

#include "common/crack_hash_request/CrackHashRequest.hpp"

#include "tp/nlohmann/json.hpp"

#include <boost/uuid/uuid.hpp>

#include <memory>

namespace ch {
namespace worker {

nlohmann::json manager_worker_addCrackHashSubRequest(
    boost::uuids::uuid crackHashRequestUuid,
    boost::uuids::uuid crackHashSubRequestUuid,
    std::shared_ptr<common::CrackHashRequest> crackHashSubRequest);

} // namespace worker
} // namespace ch