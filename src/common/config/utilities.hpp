#include "tp/spdlog/common.h"

namespace ch {
namespace common {

spdlog::level::level_enum fileLoggerLevelFromStringToEnum(const std::string &stringLevel);

spdlog::level::level_enum stdLoggerLevelFromStringToEnum(const std::string &stringLevel);

} // namespace common
} // namespace ch