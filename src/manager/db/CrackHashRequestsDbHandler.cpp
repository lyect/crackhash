#include "CrackHashRequestsDbHandler.hpp"

#include "common/crack_hash_request/SubCrackHashRequest.hpp"
#include "common/crack_hash_request/utilities.hpp"
#include "common/join_stringable.hpp"

#include "manager/config/ManagerConfig.hpp"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/string/to_string.hpp>

#include <boost/uuid/uuid_io.hpp>

namespace ch {
namespace manager {

#define LOG_CRACK_HASH_REQUEST_INFO(pREFIX)                                             \
    m_logger->info(common::joinStringable(                                              \
        pREFIX " CrackHashRequest[\n",                                                  \
        "\tCrackHashRequestUUID=", boost::uuids::to_string(crackHashRequestUuid), "\n", \
        "]"))
#define LOG_CRACK_HASH_REQUEST_WARN(pREFIX)                                             \
    m_logger->warn(common::joinStringable(                                              \
        pREFIX " CrackHashRequest[\n",                                                  \
        "\tCrackHashRequestUUID=", boost::uuids::to_string(crackHashRequestUuid), "\n", \
        "]"))

#define LOG_CRACK_HASH_SUB_REQUEST_INFO(pREFIX)                                               \
    m_logger->info(common::joinStringable(                                                    \
        pREFIX " CrackHashSubRequest[\n",                                                     \
        "\tCrackHashRequestUUID=", boost::uuids::to_string(crackHashRequestUuid), "\n",       \
        "\tCrackHashSubRequestUUID=", boost::uuids::to_string(crackHashSubRequestUuid), "\n", \
        "]"))
#define LOG_CRACK_HASH_SUB_REQUEST_WARN(pREFIX)                                               \
    m_logger->warn(common::joinStringable(                                                    \
        pREFIX " CrackHashSubRequest[\n",                                                     \
        "\tCrackHashRequestUUID=", boost::uuids::to_string(crackHashRequestUuid), "\n",       \
        "\tCrackHashSubRequestUUID=", boost::uuids::to_string(crackHashSubRequestUuid), "\n", \
        "]"))
#define LOG_CRACK_HASH_SUB_REQUEST_ERROR(pREFIX)                                              \
    m_logger->error(common::joinStringable(                                                   \
        pREFIX " CrackHashSubRequest[\n",                                                     \
        "\tCrackHashRequestUUID=", boost::uuids::to_string(crackHashRequestUuid), "\n",       \
        "\tCrackHashSubRequestUUID=", boost::uuids::to_string(crackHashSubRequestUuid), "]"))\

CrackHashRequestsDbHandler::CrackHashRequestsDbHandler()
    : m_logger{spdlog::get(config::ManagerConfig::getInstance()->loggerConfig().loggerName())}
    , m_dbName{config::ManagerConfig::getInstance()->crackHashRequestDatabaseConfig().dbName()}
    , m_dbUri(config::ManagerConfig::getInstance()->crackHashRequestDatabaseConfig().dbUri())
    , m_dbClient(m_dbUri)
    , m_db(m_dbClient[m_dbName])
    , m_session{m_dbClient.start_session()}
{
    const auto &dbConfig = config::ManagerConfig::getInstance()->crackHashRequestDatabaseConfig();
    m_crackHashSubRequestsCollection = m_db[dbConfig.crackHashSubRequestsCollection()];
    auto writeConcern = mongocxx::write_concern();
    writeConcern.nodes(3);
    m_crackHashSubRequestsCollection.write_concern(writeConcern);

    m_logger->info(common::joinStringable(
        "Connected to ReplicaSet by URI: \"",
        config::ManagerConfig::getInstance()->crackHashRequestDatabaseConfig().dbUri(), "\""));
}

CrackHashRequestsDbHandler::~CrackHashRequestsDbHandler() {
    
}

void CrackHashRequestsDbHandler::startTransaction() {
    m_transactionLock = new std::lock_guard<std::mutex>(m_transactionMutex);

    m_session.start_transaction();

    m_logger->info("Started transaction");
}

void CrackHashRequestsDbHandler::commitTransaction() {
    m_session.commit_transaction();
    
    m_logger->info("Commited transaction");

    delete m_transactionLock;
}

std::shared_ptr<common::CrackHashRequest> CrackHashRequestsDbHandler::getCrackHashSubRequest(
    boost::uuids::uuid  crackHashRequestUuid,
    boost::uuids::uuid  crackHashSubRequestUuid,
    bool               &found
) {
    const auto &dbConfig = config::ManagerConfig::getInstance()->crackHashRequestDatabaseConfig();

    auto crackHashSubRequestFilter = 
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(dbConfig.crackHashRequestUuidFieldName(),
                bsoncxx::types::b_binary {
                    .sub_type = bsoncxx::binary_sub_type::k_uuid,
                    .size = static_cast<std::uint32_t>(crackHashRequestUuid.size()),
                    .bytes = crackHashRequestUuid.data}),
            bsoncxx::builder::basic::kvp(dbConfig.crackHashSubRequestUuidFieldName(),
                bsoncxx::types::b_binary {
                    .sub_type = bsoncxx::binary_sub_type::k_uuid,
                    .size = static_cast<std::uint32_t>(crackHashSubRequestUuid.size()),
                    .bytes = crackHashSubRequestUuid.data}));

    auto crackHashRequestBson = m_crackHashSubRequestsCollection.find_one(m_session, crackHashSubRequestFilter.view());
    if (!crackHashRequestBson) {
        LOG_CRACK_HASH_SUB_REQUEST_WARN("Failed to find");
        found = false;
        return nullptr;
    }

    auto subCrackHashRequest = common::SubCrackHashRequest::createSubCrackHashRequest(
        bsoncxx::string::to_string(
            crackHashRequestBson->operator[](dbConfig.crackHashRequestHashFieldName()).get_string().value),
        static_cast<std::size_t>(
            crackHashRequestBson->operator[](dbConfig.crackHashRequestMaxLengthFieldName()).get_int64().value),
        bsoncxx::string::to_string(
            crackHashRequestBson->operator[](dbConfig.crackHashRequestAlphabetFieldName()).get_string().value),
        bsoncxx::string::to_string(
            crackHashRequestBson->operator[](dbConfig.crackHashRequestFromFieldName()).get_string().value),
        bsoncxx::string::to_string(
            crackHashRequestBson->operator[](dbConfig.crackHashRequestToFieldName()).get_string().value)
    );

    LOG_CRACK_HASH_REQUEST_INFO("Found");

    found = true;
    return subCrackHashRequest;
}

bool                                      CrackHashRequestsDbHandler::storeCrackHashSubRequest(
    boost::uuids::uuid                        crackHashRequestUuid,
    boost::uuids::uuid                        crackHashSubRequestUuid,
    std::shared_ptr<common::CrackHashRequest> crackHashSubRequest
) {
    const auto &dbConfig = config::ManagerConfig::getInstance()->crackHashRequestDatabaseConfig();

    auto crackHashSubRequestToStore = 
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(dbConfig.crackHashRequestUuidFieldName(),
                bsoncxx::types::b_binary {
                    .sub_type = bsoncxx::binary_sub_type::k_uuid,
                    .size = static_cast<std::uint32_t>(crackHashRequestUuid.size()),
                    .bytes = crackHashRequestUuid.data}),
            bsoncxx::builder::basic::kvp(dbConfig.crackHashSubRequestUuidFieldName(),
                bsoncxx::types::b_binary {
                    .sub_type = bsoncxx::binary_sub_type::k_uuid,
                    .size = static_cast<std::uint32_t>(crackHashSubRequestUuid.size()),
                    .bytes = crackHashSubRequestUuid.data}),
            bsoncxx::builder::basic::kvp(dbConfig.crackHashRequestHashFieldName(),
                bsoncxx::types::b_string(crackHashSubRequest->hash())),
            bsoncxx::builder::basic::kvp(dbConfig.crackHashRequestMaxLengthFieldName(),
                bsoncxx::types::b_int64{
                    .value = static_cast<std::int64_t>(crackHashSubRequest->maxLength())}),
            bsoncxx::builder::basic::kvp(dbConfig.crackHashRequestAlphabetFieldName(),
                 bsoncxx::types::b_string(crackHashSubRequest->alphabet())),
            bsoncxx::builder::basic::kvp(dbConfig.crackHashRequestFromFieldName(),
                bsoncxx::types::b_string(crackHashSubRequest->from())),
            bsoncxx::builder::basic::kvp(dbConfig.crackHashRequestToFieldName(),
                bsoncxx::types::b_string(crackHashSubRequest->to())),
            bsoncxx::builder::basic::kvp(dbConfig.crackHashRequestResultFieldName(),
                bsoncxx::types::b_array()),
            bsoncxx::builder::basic::kvp(dbConfig.crackHashRequestStatusFieldName(),
                bsoncxx::types::b_int32(
                    common::crackHashRequestStatusToInt32(common::CrackHashRequestStatus::IN_PROGRESS))));

    if (!m_crackHashSubRequestsCollection.insert_one(m_session, crackHashSubRequestToStore.view())) {
        LOG_CRACK_HASH_SUB_REQUEST_ERROR("Failed to store");
        return false;
    }

    LOG_CRACK_HASH_SUB_REQUEST_INFO("Stored");
    return true;
}

common::CrackHashRequestStatus CrackHashRequestsDbHandler::getCrackHashSubRequestStatus(
    boost::uuids::uuid  crackHashRequestUuid,
    boost::uuids::uuid  crackHashSubRequestUuid,
    bool               &found
) {
    const auto &dbConfig = config::ManagerConfig::getInstance()->crackHashRequestDatabaseConfig();

    auto crackHashSubRequestFilter = 
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(dbConfig.crackHashRequestUuidFieldName(),
                bsoncxx::types::b_binary {
                    .sub_type = bsoncxx::binary_sub_type::k_uuid,
                    .size = static_cast<std::uint32_t>(crackHashRequestUuid.size()),
                    .bytes = crackHashRequestUuid.data}),
            bsoncxx::builder::basic::kvp(dbConfig.crackHashSubRequestUuidFieldName(),
                bsoncxx::types::b_binary {
                    .sub_type = bsoncxx::binary_sub_type::k_uuid,
                    .size = static_cast<std::uint32_t>(crackHashSubRequestUuid.size()),
                    .bytes = crackHashSubRequestUuid.data}));

    auto crackHashRequestBson = m_crackHashSubRequestsCollection.find_one(crackHashSubRequestFilter.view());
    if (!crackHashRequestBson) {
        LOG_CRACK_HASH_SUB_REQUEST_WARN("Failed to get status: Not found");
        found = false;
        return common::CrackHashRequestStatus::UNKNOWN;
    }

    auto crackHashSubRequestStatus = common::int32ToCrackHashRequestStatus(
        static_cast<std::int32_t>(
            crackHashRequestBson->operator[](dbConfig.crackHashRequestStatusFieldName()).get_int32().value));

    LOG_CRACK_HASH_SUB_REQUEST_INFO("Got status: Found");

    found = true;
    return crackHashSubRequestStatus;
}

bool                           CrackHashRequestsDbHandler::setCrackHashSubRequestStatus(
    boost::uuids::uuid             crackHashRequestUuid,
    boost::uuids::uuid             crackHashSubRequestUuid,
    common::CrackHashRequestStatus crackHashSubRequestStatus
) {
    const auto &dbConfig = config::ManagerConfig::getInstance()->crackHashRequestDatabaseConfig();

    auto crackHashSubRequestFilter = 
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(dbConfig.crackHashRequestUuidFieldName(),
                bsoncxx::types::b_binary {
                    .sub_type = bsoncxx::binary_sub_type::k_uuid,
                    .size = static_cast<std::uint32_t>(crackHashRequestUuid.size()),
                    .bytes = crackHashRequestUuid.data}),
            bsoncxx::builder::basic::kvp(dbConfig.crackHashSubRequestUuidFieldName(),
                bsoncxx::types::b_binary {
                    .sub_type = bsoncxx::binary_sub_type::k_uuid,
                    .size = static_cast<std::uint32_t>(crackHashSubRequestUuid.size()),
                    .bytes = crackHashSubRequestUuid.data}));

    auto crackHashSubRequestUpdate = bsoncxx::builder::stream::document();

    crackHashSubRequestUpdate << "$set" << bsoncxx::builder::stream::open_document
                                        << dbConfig.crackHashRequestStatusFieldName()
                                        <<     bsoncxx::types::b_int32(
                                                   common::crackHashRequestStatusToInt32(
                                                       crackHashSubRequestStatus))
                                        << bsoncxx::builder::stream::close_document;

    if (
        !m_crackHashSubRequestsCollection.update_one(
            m_session,
            crackHashSubRequestFilter.view(),
            crackHashSubRequestUpdate.view())
    ) {
        LOG_CRACK_HASH_SUB_REQUEST_ERROR("Failed to set status of");
        return false;
    }

    LOG_CRACK_HASH_SUB_REQUEST_INFO("Set status of");
    return true;
}

std::vector<std::string> CrackHashRequestsDbHandler::getCrackHashSubRequestResult(
    boost::uuids::uuid  crackHashRequestUuid,
    boost::uuids::uuid  crackHashSubRequestUuid,
    bool               &found
) {
    const auto &dbConfig = config::ManagerConfig::getInstance()->crackHashRequestDatabaseConfig();

    auto crackHashSubRequestFilter = 
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(dbConfig.crackHashRequestUuidFieldName(),
                bsoncxx::types::b_binary {
                    .sub_type = bsoncxx::binary_sub_type::k_uuid,
                    .size = static_cast<std::uint32_t>(crackHashRequestUuid.size()),
                    .bytes = crackHashRequestUuid.data}),
            bsoncxx::builder::basic::kvp(dbConfig.crackHashSubRequestUuidFieldName(),
                bsoncxx::types::b_binary {
                    .sub_type = bsoncxx::binary_sub_type::k_uuid,
                    .size = static_cast<std::uint32_t>(crackHashSubRequestUuid.size()),
                    .bytes = crackHashSubRequestUuid.data}));
    
    auto crackHashRequestBson = m_crackHashSubRequestsCollection.find_one(crackHashSubRequestFilter.view());
    if (!crackHashRequestBson) {
        LOG_CRACK_HASH_SUB_REQUEST_WARN("Failed to get result: Not found");
        found = false;
        return {};
    }

    std::vector<std::string> toReturn;

    const auto &bsonArray = 
        crackHashRequestBson->operator[](dbConfig.crackHashRequestResultFieldName()).get_array().value;

    for (const auto &resultEntry: bsonArray) {
        toReturn.push_back(bsoncxx::string::to_string(resultEntry.get_string().value));
    }

    LOG_CRACK_HASH_SUB_REQUEST_WARN("Got result: Found");

    found = true;
    return toReturn;
}

bool                     CrackHashRequestsDbHandler::setCrackHashSubRequestResult(
    boost::uuids::uuid        crackHashRequestUuid,
    boost::uuids::uuid        crackHashSubRequestUuid,
    std::vector<std::string>  crackHashSubRequestResult
) {
    const auto &dbConfig = config::ManagerConfig::getInstance()->crackHashRequestDatabaseConfig();

    auto crackHashSubRequestFilter = 
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(dbConfig.crackHashRequestUuidFieldName(),
                bsoncxx::types::b_binary {
                    .sub_type = bsoncxx::binary_sub_type::k_uuid,
                    .size = static_cast<std::uint32_t>(crackHashRequestUuid.size()),
                    .bytes = crackHashRequestUuid.data}),
            bsoncxx::builder::basic::kvp(dbConfig.crackHashSubRequestUuidFieldName(),
                bsoncxx::types::b_binary {
                    .sub_type = bsoncxx::binary_sub_type::k_uuid,
                    .size = static_cast<std::uint32_t>(crackHashSubRequestUuid.size()),
                    .bytes = crackHashSubRequestUuid.data}));

    bsoncxx::builder::basic::array resultBsonArray;
    for (const auto &resultEntry: crackHashSubRequestResult) {
        resultBsonArray.append(resultEntry);
    }

    auto crackHashSubRequestUpdate = bsoncxx::builder::stream::document();

    crackHashSubRequestUpdate << "$set" << bsoncxx::builder::stream::open_document
                                        << dbConfig.crackHashRequestResultFieldName()
                                        <<     resultBsonArray
                                        << dbConfig.crackHashRequestStatusFieldName()
                                        <<     bsoncxx::types::b_int32(
                                                   common::crackHashRequestStatusToInt32(
                                                       common::CrackHashRequestStatus::READY))
                                        << bsoncxx::builder::stream::close_document;

    if (
        !m_crackHashSubRequestsCollection.update_one(
            m_session,
            crackHashSubRequestFilter.view(),
            crackHashSubRequestUpdate.view())
    ) {
        LOG_CRACK_HASH_SUB_REQUEST_ERROR("Failed to set result of");
        return false;
    }

    LOG_CRACK_HASH_SUB_REQUEST_INFO("Set result of");
    return true;
}

common::CrackHashRequestStatus CrackHashRequestsDbHandler::getCrackHashRequestStatus(
    boost::uuids::uuid  crackHashRequestUuid,
    bool               &found
) {
    const auto *managerConfig = config::ManagerConfig::getInstance();
    const auto &dbConfig = managerConfig->crackHashRequestDatabaseConfig();

    auto crackHashSubRequestFilter = 
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(dbConfig.crackHashRequestUuidFieldName(),
                bsoncxx::types::b_binary {
                    .sub_type = bsoncxx::binary_sub_type::k_uuid,
                    .size = static_cast<std::uint32_t>(crackHashRequestUuid.size()),
                    .bytes = crackHashRequestUuid.data}));

    auto cursor = m_crackHashSubRequestsCollection.find(crackHashSubRequestFilter.view());

    if (cursor.begin() == cursor.end()) {
        LOG_CRACK_HASH_REQUEST_WARN("Failed to get status: Not found");
        found = false;
        return common::CrackHashRequestStatus::UNKNOWN;
    }

    std::size_t nInProgress = 0;
    std::size_t nTimeout = 0;
    std::size_t nReady = 0;
    std::size_t nUnknown = 0;

    for (const auto &crackHashSubRequestsEntry: cursor) {
        auto status = common::int32ToCrackHashRequestStatus(
            crackHashSubRequestsEntry[dbConfig.crackHashRequestStatusFieldName()].get_int32().value);
        switch (status) {
            case common::CrackHashRequestStatus::IN_PROGRESS:
                ++nInProgress;
                break;
            case common::CrackHashRequestStatus::TIMEOUT:
                ++nTimeout;
                break;
            case common::CrackHashRequestStatus::READY:
                ++nReady;
                break;
            default:
                ++nUnknown;
                break;
        }
    }

    common::CrackHashRequestStatus crackHashRequestStatus;
    if (nUnknown > 0) {
        crackHashRequestStatus = common::CrackHashRequestStatus::UNKNOWN;
    }
    else if (nTimeout > 0) {
        crackHashRequestStatus = common::CrackHashRequestStatus::TIMEOUT;
    }
    else if (nInProgress > 0 && nInProgress + nReady == managerConfig->splitInto()) {
        crackHashRequestStatus = common::CrackHashRequestStatus::IN_PROGRESS;
    }
    else if (nReady == managerConfig->splitInto()) {
        crackHashRequestStatus = common::CrackHashRequestStatus::READY;
    }
    else {
        LOG_CRACK_HASH_REQUEST_WARN("Failed to get status: No suitable conditions for");
        found = false;
        return common::CrackHashRequestStatus::UNKNOWN;
    }

    LOG_CRACK_HASH_REQUEST_INFO("Got status: Found");

    found = true;
    return crackHashRequestStatus;
}

std::vector<std::string> CrackHashRequestsDbHandler::getCrackHashRequestResult(
    boost::uuids::uuid  crackHashRequestUuid,
    bool               &found
) {
    const auto *managerConfig = config::ManagerConfig::getInstance();
    const auto &dbConfig = managerConfig->crackHashRequestDatabaseConfig();

    auto crackHashSubRequestFilter = 
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(dbConfig.crackHashRequestUuidFieldName(),
                bsoncxx::types::b_binary {
                    .sub_type = bsoncxx::binary_sub_type::k_uuid,
                    .size = static_cast<std::uint32_t>(crackHashRequestUuid.size()),
                    .bytes = crackHashRequestUuid.data}));

    auto cursor = m_crackHashSubRequestsCollection.find(crackHashSubRequestFilter.view());

    if (cursor.begin() == cursor.end()) {
        LOG_CRACK_HASH_REQUEST_WARN("Failed to get result: Not found");
        found = false;
        return {};
    }

    std::vector<std::string> toReturn;

    for (const auto &crackHashSubRequestsEntry: cursor) {
        auto result = crackHashSubRequestsEntry[dbConfig.crackHashRequestResultFieldName()].get_array().value;
        for (const auto &resultEntry: result) {
            toReturn.push_back(bsoncxx::string::to_string(resultEntry.get_string().value));
        }
    }

    LOG_CRACK_HASH_REQUEST_WARN("Got result: Found");

    found = true;
    return toReturn;
}

} // namespace manager
} // namespace ch