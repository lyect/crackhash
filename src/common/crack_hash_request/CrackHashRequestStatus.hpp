#pragma once

namespace ch {
namespace common {

enum class CrackHashRequestStatus {
    IN_PROGRESS,
    TIMEOUT,
    READY,
    UNKNOWN
};

} // namespace common
} // namespace ch