#pragma once

#include "common/crack_hash_request/CrackHashRequestStatus.hpp"
#include "common/crack_hash_request/CrackHashRequestType.hpp"

#include <string>

namespace ch {
namespace common {

std::string crackHashRequestStatusToString(CrackHashRequestStatus status);
std::int32_t crackHashRequestStatusToInt32(CrackHashRequestStatus status);
CrackHashRequestStatus int32ToCrackHashRequestStatus(std::int32_t status);

std::string crackHashRequestTypeToString(CrackHashRequestType type);

} // namespace common
} // namespace ch