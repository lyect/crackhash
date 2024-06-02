#include "utilities.hpp"

#include <stdexcept>

namespace ch {
namespace common {

std::string crackHashRequestStatusToString(CrackHashRequestStatus status) {
    switch (status) {
        case CrackHashRequestStatus::IN_PROGRESS:
            return "IN_PROGRESS";
        case CrackHashRequestStatus::TIMEOUT:
            return "TIMEOUT";
        case CrackHashRequestStatus::READY:
            return "READY";
        default:
            throw std::runtime_error("Failed status -> str: Unknown CrackHashRequestStatus");
    }
}

std::int32_t crackHashRequestStatusToInt32(CrackHashRequestStatus status) {
    switch (status) {
        case CrackHashRequestStatus::IN_PROGRESS:
            return 0;
        case CrackHashRequestStatus::TIMEOUT:
            return 1;
        case CrackHashRequestStatus::READY:
            return 2;
        default:
            throw std::runtime_error("Failed status -> int32: Unknown CrackHashRequestStatus");
    }
}

CrackHashRequestStatus int32ToCrackHashRequestStatus(std::int32_t status) {
    switch (status) {
        case 0:
            return CrackHashRequestStatus::IN_PROGRESS;
        case 1:
            return CrackHashRequestStatus::TIMEOUT;
        case 2:
            return CrackHashRequestStatus::READY;
        default:
            throw std::runtime_error("Failed int32 -> status: Unknown CrackHashRequestStatus");
    }
}

std::string crackHashRequestTypeToString(CrackHashRequestType type) {
    switch (type) {
        case CrackHashRequestType::FULL: {
            return "FullCrackHashRequest";
        }
        case CrackHashRequestType::SPLITTED: {
            return "SplittedCrackHashRequest";
        }
        case CrackHashRequestType::SUB: {
            return "SubCrackHashRequest";
        }
        default: {
            throw std::runtime_error("Failed type -> str: Unknown CrackHashRequestType");
        }
    }
}

} // namespace common
} // namespace ch