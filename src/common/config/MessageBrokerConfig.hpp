#pragma once

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

class MessageBrokerConfig {

    friend manager::config::ManagerConfigParser;
    friend worker::config::WorkerConfigParser;
    friend class CommonConfig;

public:

    const std::string &hostname() const {
        return m_hostname;
    }

    std::uint16_t port() const {
        return m_port;
    }
    
    const std::string &user() const {
        return m_user;
    }

    const std::string &password() const {
        return m_password;
    }

    const std::string &toWorkersQueue() const {
        return m_toWorkersQueue;
    }
    
    const std::string &toWorkersRoutingKey() const {
        return m_toWorkersRoutingKey;
    }
    
    const std::string &toWorkersExchangePoint() const {
        return m_toWorkersExchangePoint;
    }

    const std::string &fromWorkersQueue() const {
        return m_fromWorkersQueue;
    }
    
    const std::string &fromWorkersRoutingKey() const {
        return m_fromWorkersRoutingKey;
    }
    
    const std::string &fromWorkersExchangePoint() const {
        return m_fromWorkersExchangePoint;
    }

    std::uint16_t publishInterval() const {
        return m_publishInterval;
    }

    std::uint16_t consumeInterval() const {
        return m_consumeInterval;
    }

private:

    std::string   m_hostname;
    std::uint16_t m_port;
    std::string   m_user;
    std::string   m_password;

    std::string m_toWorkersQueue;
    std::string m_toWorkersRoutingKey;
    std::string m_toWorkersExchangePoint;
    
    std::string m_fromWorkersQueue;
    std::string m_fromWorkersRoutingKey;
    std::string m_fromWorkersExchangePoint;

    std::uint16_t m_publishInterval;
    std::uint16_t m_consumeInterval;
};

} // namespace config
} // namespace manager
} // namespace ch