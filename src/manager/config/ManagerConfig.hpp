#pragma once

#include "common/config/CommonConfig.hpp"

#include "manager/config/CrackHashRequestDatabaseConfig.hpp"
#include "manager/config/IncomingHttpConnectionConfig.hpp"

namespace ch {
namespace manager {
namespace config {

class ManagerConfig : public common::config::CommonConfig {

    friend class ManagerConfigParser;

public:

    static ManagerConfig *getInstance() {
        return static_cast<ManagerConfig *>(common::config::CommonConfig::getInstance());
    }

    std::uint16_t startDelay() const {
        return m_startDelay;
    }

    std::size_t nThreads() const {
        return m_nThreads;
    }

    std::size_t maxLength() const {
        return m_maxLength;
    }

    const std::string &alphabet() const {
        return m_alphabet;
    }

    std::uint16_t crackHashRequestTimeout() const {
        return m_crackHashRequestTimeout;
    }
    
    std::uint16_t splitInto() const {
        return m_splitInto;
    }

    const IncomingHttpConnectionConfig &incomingHttpConnectionConfig() const {
        return m_incomingHttpConnectionConfig;
    }
    
    const CrackHashRequestDatabaseConfig &crackHashRequestDatabaseConfig() const {
        return m_crackHashRequestDatabaseConfig;
    }

private:

    std::uint16_t                  m_startDelay;
    std::size_t                    m_nThreads;
    std::size_t                    m_maxLength;
    std::string                    m_alphabet;
    std::uint16_t                  m_crackHashRequestTimeout;
    std::uint16_t                  m_splitInto;
    CrackHashRequestDatabaseConfig m_crackHashRequestDatabaseConfig;
    IncomingHttpConnectionConfig   m_incomingHttpConnectionConfig;
};

} // namespace config
} // namespace manager
} // namespace ch