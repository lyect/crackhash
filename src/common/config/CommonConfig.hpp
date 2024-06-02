#pragma once

#include "common/config/LoggerConfig.hpp"
#include "common/config/MessageBrokerConfig.hpp"

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

class CommonConfig {

    friend manager::config::ManagerConfigParser;
    friend worker::config::WorkerConfigParser;

public:

    static CommonConfig *getInstance() {
        return m_instance;
    }

    static void reset(CommonConfig *instance) {
        delete m_instance;
        m_instance = instance;
    }

    const LoggerConfig &loggerConfig() const {
        return m_loggerConfig;
    }

    const MessageBrokerConfig &messageBrokerConfig() const {
        return m_messageBrokerConfig;
    }

protected:

    CommonConfig() = default;

private:

    CommonConfig(const CommonConfig &) = delete;
    CommonConfig(      CommonConfig &) = delete;
    CommonConfig &operator=(const CommonConfig &) = delete;
    CommonConfig &operator=(      CommonConfig &) = delete;

    static CommonConfig *m_instance;

    LoggerConfig        m_loggerConfig;
    MessageBrokerConfig m_messageBrokerConfig;
};

} // namespace config
} // namespace common
} // namespace ch