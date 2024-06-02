#pragma once

#include "spdlog/common.h"

namespace ch {

namespace manager {
namespace config {

class ManagerConfigParser;

} // namespace config
} // namespace manager

namespace worker {
namespace config {

class WorkerConfigParser;

} // namespace config
} // namespace worker

namespace common {
namespace config {

class StdLoggerConfig {

    friend manager::config::ManagerConfigParser;
    friend worker::config::WorkerConfigParser;
    friend class CommonConfig;

public:

    spdlog::level::level_enum level() const {
        return m_level;
    }

private:

    StdLoggerConfig() = default;

    spdlog::level::level_enum m_level;
};

} // namespace config
} // namespace common
} // namespace ch