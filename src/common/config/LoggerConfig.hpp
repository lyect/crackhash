#pragma once

#include "common/config/FileLoggerConfig.hpp"
#include "common/config/StdLoggerConfig.hpp"

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

class LoggerConfig {

    friend manager::config::ManagerConfigParser;
    friend worker::config::WorkerConfigParser;
    friend class CommonConfig;
    
public:

    ~LoggerConfig() {
        delete m_fileLoggerConfig;
        delete m_stdLoggerConfig;
    }

    bool fileLoggerConfigPresented() const {
        return m_fileLoggerConfig != nullptr;
    }

    bool stdLoggerConfigPresented() const {
        return m_stdLoggerConfig != nullptr;
    }

    const FileLoggerConfig &fileLoggerConfig() const {
        return *m_fileLoggerConfig;
    }

    const StdLoggerConfig &stdLoggerConfig() const {
        return *m_stdLoggerConfig;
    }

    const std::string &loggerName() const {
        return m_loggerName;
    }

    std::size_t queueSize() const {
        return m_queueSize;
    }

    std::size_t nThreads() const {
        return m_nThreads;
    }

private:

    LoggerConfig() {
        m_fileLoggerConfig = nullptr;
        m_stdLoggerConfig = nullptr;
    }

    FileLoggerConfig *m_fileLoggerConfig;
    StdLoggerConfig  *m_stdLoggerConfig;
    std::string       m_loggerName;
    std::size_t       m_queueSize;
    std::size_t       m_nThreads;
};

} // namespace config
} // namespace common
} // namespace ch