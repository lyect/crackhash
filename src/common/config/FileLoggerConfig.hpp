#pragma once

#include "spdlog/common.h"

#include <string>

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

class FileLoggerConfig {

    friend manager::config::ManagerConfigParser;
    friend worker::config::WorkerConfigParser;
    friend class CommonConfig;

public:

    spdlog::level::level_enum level() const {
        return m_level;
    }

    const std::string &loggingFilePath() const {
        return m_loggingFilePath;
    }

    bool truncate() const {
        return m_truncate;
    }

private:

    FileLoggerConfig() = default;

    spdlog::level::level_enum m_level;
    std::string               m_loggingFilePath;
    bool                      m_truncate;
};

} // namespace config
} // namespace common
} // namespace ch