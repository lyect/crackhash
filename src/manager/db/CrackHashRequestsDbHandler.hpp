#pragma once

#include "common/crack_hash_request/CrackHashRequest.hpp"
#include "common/crack_hash_request/CrackHashRequestStatus.hpp"

#include "tp/spdlog/spdlog.h"

#include <boost/uuid/uuid.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/client_session.hpp>

#include <memory>
#include <vector>
#include <string>

namespace ch {
namespace manager {

class CrackHashRequestsDbHandler {

public:

    CrackHashRequestsDbHandler();

    ~CrackHashRequestsDbHandler();

    void startTransaction();
    void commitTransaction();

    std::shared_ptr<common::CrackHashRequest> getCrackHashSubRequest(
        boost::uuids::uuid  crackHashRequestUuid,
        boost::uuids::uuid  crackHashSubRequestUuid,
        bool               &found);
    bool                                      storeCrackHashSubRequest(
        boost::uuids::uuid                        crackHashRequestUuid,
        boost::uuids::uuid                        crackHashSubRequestUuid,
        std::shared_ptr<common::CrackHashRequest> crackHashSubRequest);
    
    common::CrackHashRequestStatus getCrackHashSubRequestStatus(
        boost::uuids::uuid  crackHashRequestUuid,
        boost::uuids::uuid  crackHashSubRequestUuid,
        bool               &found);
    bool                           setCrackHashSubRequestStatus(
        boost::uuids::uuid              crackHashRequestUuid,
        boost::uuids::uuid              crackHashSubRequestUuid,
        common::CrackHashRequestStatus  crackHashSubRequestStatus);

    std::vector<std::string> getCrackHashSubRequestResult(
        boost::uuids::uuid  crackHashRequestUuid,
        boost::uuids::uuid  crackHashSubRequestUuid,
        bool               &found);
    bool                     setCrackHashSubRequestResult(
        boost::uuids::uuid        crackHashRequestUuid,
        boost::uuids::uuid        crackHashSubRequestUuid,
        std::vector<std::string>  crackHashSubRequestResult);

    common::CrackHashRequestStatus getCrackHashRequestStatus(
        boost::uuids::uuid  crackHashRequestUuid,
        bool               &found);

    std::vector<std::string> getCrackHashRequestResult(
        boost::uuids::uuid  crackHashRequestUuid,
        bool               &found);

private:

    std::shared_ptr<spdlog::logger> m_logger;

    std::string m_dbName;

    mongocxx::uri            m_dbUri;
    mongocxx::client         m_dbClient;
    mongocxx::database       m_db;
    mongocxx::client_session m_session;
    mongocxx::collection     m_crackHashSubRequestsCollection;

    std::mutex                   m_transactionMutex;
    std::lock_guard<std::mutex> *m_transactionLock;
};

} // namespace manager
} // namespace ch