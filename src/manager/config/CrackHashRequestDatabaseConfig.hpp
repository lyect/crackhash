#pragma once

#include <string>

namespace ch {
namespace manager {
namespace config {

class CrackHashRequestDatabaseConfig {

    friend class ManagerConfigParser;
    friend class ManagerConfig;

public:

    const std::string &dbUri() const {
        return m_dbUri;
    }

    const std::string &dbName() const {
        return m_dbName;
    }

    const std::string &crackHashSubRequestsCollection() const {
        return m_crackHashSubRequestsCollection;
    }

    const std::string &crackHashRequestUuidFieldName() const {
        return m_crackHashRequestUuidFieldName;
    }

    const std::string &crackHashSubRequestUuidFieldName() const {
        return m_crackHashSubRequestUuidFieldName;
    }

    const std::string &crackHashRequestHashFieldName() const {
        return m_crackHashRequestHashFieldName;
    }

    const std::string &crackHashRequestMaxLengthFieldName() const {
        return m_crackHashRequestMaxLengthFieldName;
    }

    const std::string &crackHashRequestAlphabetFieldName() const {
        return m_crackHashRequestAlphabetFieldName;
    }

    const std::string &crackHashRequestFromFieldName() const {
        return m_crackHashRequestFromFieldName;
    }

    const std::string &crackHashRequestToFieldName() const {
        return m_crackHashRequestToFieldName;
    }

    const std::string &crackHashRequestResultFieldName() const {
        return m_crackHashRequestResultFieldName;
    }

    const std::string &crackHashRequestStatusFieldName() const {
        return m_crackHashRequestStatusFieldName;
    }

private:

    CrackHashRequestDatabaseConfig() = default;

    std::string m_dbUri;
    std::string m_dbName;
    std::string m_crackHashSubRequestsCollection;
    std::string m_crackHashRequestUuidFieldName;
    std::string m_crackHashSubRequestUuidFieldName;
    std::string m_crackHashRequestHashFieldName;
    std::string m_crackHashRequestMaxLengthFieldName;
    std::string m_crackHashRequestAlphabetFieldName;
    std::string m_crackHashRequestFromFieldName;
    std::string m_crackHashRequestToFieldName;
    std::string m_crackHashRequestResultFieldName;
    std::string m_crackHashRequestStatusFieldName;
};

} // namespace config
} // namespace manager
} // namespace ch