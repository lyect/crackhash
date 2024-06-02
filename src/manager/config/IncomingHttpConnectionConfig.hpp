#pragma once

#include <string>

namespace ch {
namespace manager {
namespace config {

class IncomingHttpConnectionConfig {

    friend class ManagerConfigParser;
    friend class ManagerConfig;

public:

    const std::string &ipAddress() const {
        return m_ipAddress;
    }

    std::uint16_t ipPort() const {
        return m_ipPort;
    }

    std::size_t readBufferSize() const {
        return m_readBufferSize;
    }

    std::uint16_t timeout() const {
        return m_timeout;
    }

private:

    IncomingHttpConnectionConfig() = default;

    std::string   m_ipAddress;
    std::uint16_t m_ipPort;
    std::size_t   m_readBufferSize;
    std::uint16_t m_timeout;
};

} // namespace config
} // namespace manager
} // namespace ch