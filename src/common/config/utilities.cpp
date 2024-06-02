#include "utilities.hpp"

#include <iostream>

namespace ch {
namespace common {

spdlog::level::level_enum fileLoggerLevelFromStringToEnum(const std::string &stringLevel) {
    if (stringLevel == "trace") {
        return spdlog::level::trace;
    }
    else if (stringLevel == "debug") {
        return spdlog::level::debug;
    }
    else if (stringLevel == "info") {
        return spdlog::level::info;
    }
    else if (stringLevel == "warn") {
        return spdlog::level::warn;
    }
    else if (stringLevel == "err") {
        return spdlog::level::err;
    }
    else if (stringLevel == "critical") {
        return spdlog::level::critical;
    }
    else if (stringLevel == "off") {
        return spdlog::level::off;
    }
    else {
        std::cerr << "Unknown file logger's level config: \"" \
            << stringLevel << "\". Setting \"info\" as default." << std::endl;
        return spdlog::level::info;
    }
}

spdlog::level::level_enum stdLoggerLevelFromStringToEnum(const std::string &stringLevel) {
    if (stringLevel == "trace") {
        return spdlog::level::trace;
    }
    else if (stringLevel == "debug") {
        return spdlog::level::debug;
    }
    else if (stringLevel == "info") {
        return spdlog::level::info;
    }
    else if (stringLevel == "warn") {
        return spdlog::level::warn;
    }
    else if (stringLevel == "err") {
        return spdlog::level::err;
    }
    else if (stringLevel == "critical") {
        return spdlog::level::critical;
    }
    else if (stringLevel == "off") {
        return spdlog::level::off;
    }
    else {
        std::cerr << "Unknown std logger's level config: \"" \
            << stringLevel << "\". Setting \"info\" as default." << std::endl;
        return spdlog::level::info;
    }
}

} // namespace common
} // namespace ch