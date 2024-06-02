#include "ManagerConfigParser.hpp"

#include "common/config/utilities.hpp"

#include "manager/config/ManagerConfig.hpp"

#include <iostream>
#include <fstream>
#include <cstring>
#include <cerrno>

namespace ch {
namespace manager {
namespace config {

bool ManagerConfigParser::read(const std::string &configFilePath) {

    std::ifstream ifs(configFilePath);

    if (!ifs.is_open()) {
        std::cerr << "Failed to open config file: " << strerror(errno) << std::endl;
        return false;
    }

    ManagerConfig *managerConfig = new ManagerConfig();
    common::config::CommonConfig::reset(managerConfig);

    nlohmann::json jsonManagerConfig;
    try {
        jsonManagerConfig = nlohmann::json::parse(ifs);
        parseLoggerConfig(jsonManagerConfig);
        parseIncomingHttpConnectionConfig(jsonManagerConfig);
        parseMessageBrokerConfig(jsonManagerConfig);
        parseCrackHashRequestDatabaseConfig(jsonManagerConfig);
        parseManagerParameters(jsonManagerConfig);
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        ifs.close();
        return false;
    }

    ifs.close();
    return true;
}

void ManagerConfigParser::parseLoggerConfig(const nlohmann::json &jsonManagerConfig) {
    auto *managerConfig = ManagerConfig::getInstance();

    const auto &jsonManagerLoggerConfig = jsonManagerConfig["logger"];

    managerConfig->m_loggerConfig.m_loggerName = \
        jsonManagerLoggerConfig["name"].get<std::string>();
    managerConfig->m_loggerConfig.m_queueSize = \
        jsonManagerLoggerConfig["queue_size"].get<std::size_t>();
    managerConfig->m_loggerConfig.m_nThreads = \
        jsonManagerLoggerConfig["n_threads"].get<std::size_t>();

    if (jsonManagerLoggerConfig.contains("file_logger")) {
        auto &jsonFileLoggerConfig = jsonManagerLoggerConfig["file_logger"];

        managerConfig->m_loggerConfig.m_fileLoggerConfig = new common::config::FileLoggerConfig();
        managerConfig->m_loggerConfig.m_fileLoggerConfig->m_loggingFilePath = \
            jsonFileLoggerConfig["path"].get<std::string>();
        managerConfig->m_loggerConfig.m_fileLoggerConfig->m_truncate = \
            jsonFileLoggerConfig["truncate"].get<bool>();
        managerConfig->m_loggerConfig.m_fileLoggerConfig->m_level = \
            common::fileLoggerLevelFromStringToEnum(jsonFileLoggerConfig["level"].get<std::string>()); 
    }

    if (jsonManagerLoggerConfig.contains("std_logger")) {
        auto &jsonStdLoggerConfig = jsonManagerLoggerConfig["std_logger"];

        managerConfig->m_loggerConfig.m_stdLoggerConfig = new common::config::StdLoggerConfig();
        managerConfig->m_loggerConfig.m_stdLoggerConfig->m_level = \
           common::stdLoggerLevelFromStringToEnum(jsonStdLoggerConfig["level"].get<std::string>()); 
    }
}

void ManagerConfigParser::parseIncomingHttpConnectionConfig(const nlohmann::json &jsonManagerConfig) {
    auto *managerConfig = ManagerConfig::getInstance();

    const auto &jsonManagerIncomingHttpConnectionConfig = \
        jsonManagerConfig["incoming_http_connection"];

    managerConfig->m_incomingHttpConnectionConfig.m_ipAddress = \
        jsonManagerIncomingHttpConnectionConfig["ip_address"].get<std::string>();
    managerConfig->m_incomingHttpConnectionConfig.m_ipPort = \
        jsonManagerIncomingHttpConnectionConfig["ip_port"].get<std::uint16_t>();
    managerConfig->m_incomingHttpConnectionConfig.m_readBufferSize = \
        jsonManagerIncomingHttpConnectionConfig["read_buffer_size"].get<std::size_t>();
    managerConfig->m_incomingHttpConnectionConfig.m_timeout = \
        jsonManagerIncomingHttpConnectionConfig["timeout"].get<std::uint16_t>();
}

void ManagerConfigParser::parseMessageBrokerConfig(const nlohmann::json &jsonManagerConfig) {
    auto *managerConfig = ManagerConfig::getInstance();

    const auto &jsonMessageBrokerConfig = jsonManagerConfig["message_broker"];
    managerConfig->m_messageBrokerConfig.m_hostname = \
        jsonMessageBrokerConfig["hostname"].get<std::string>();
    managerConfig->m_messageBrokerConfig.m_port = \
        jsonMessageBrokerConfig["port"].get<std::uint16_t>();
    managerConfig->m_messageBrokerConfig.m_user = \
        jsonMessageBrokerConfig["user"].get<std::string>();
    managerConfig->m_messageBrokerConfig.m_password = \
        jsonMessageBrokerConfig["password"].get<std::string>();
    managerConfig->m_messageBrokerConfig.m_toWorkersQueue = \
        jsonMessageBrokerConfig["to_workers_queue"].get<std::string>();
    managerConfig->m_messageBrokerConfig.m_toWorkersRoutingKey = \
        jsonMessageBrokerConfig["to_workers_routing_key"].get<std::string>();
    managerConfig->m_messageBrokerConfig.m_toWorkersExchangePoint = \
        jsonMessageBrokerConfig["to_workers_exchange_point"].get<std::string>();
    managerConfig->m_messageBrokerConfig.m_fromWorkersQueue = \
        jsonMessageBrokerConfig["from_workers_queue"].get<std::string>();
    managerConfig->m_messageBrokerConfig.m_fromWorkersRoutingKey = \
        jsonMessageBrokerConfig["from_workers_routing_key"].get<std::string>();
    managerConfig->m_messageBrokerConfig.m_fromWorkersExchangePoint = \
        jsonMessageBrokerConfig["from_workers_exchange_point"].get<std::string>();
    managerConfig->m_messageBrokerConfig.m_publishInterval = \
        jsonMessageBrokerConfig["publish_interval"].get<std::uint16_t>();
    managerConfig->m_messageBrokerConfig.m_consumeInterval = \
        jsonMessageBrokerConfig["consume_interval"].get<std::uint16_t>();
}

void ManagerConfigParser::parseCrackHashRequestDatabaseConfig(const nlohmann::json &jsonManagerConfig) {
    auto *managerConfig = ManagerConfig::getInstance();

    const auto &jsonCrackHashRequestDatabaseConfig = jsonManagerConfig["crack_hash_request_database"];
    managerConfig->m_crackHashRequestDatabaseConfig.m_dbUri = \
        jsonCrackHashRequestDatabaseConfig["db_uri"].get<std::string>();
    managerConfig->m_crackHashRequestDatabaseConfig.m_dbName = \
        jsonCrackHashRequestDatabaseConfig["db_name"].get<std::string>();
    managerConfig->m_crackHashRequestDatabaseConfig.m_crackHashSubRequestsCollection = \
        jsonCrackHashRequestDatabaseConfig["crack_hash_sub_requests_collection"].get<std::string>();
    managerConfig->m_crackHashRequestDatabaseConfig.m_crackHashRequestUuidFieldName = \
        jsonCrackHashRequestDatabaseConfig["crack_hash_request_uuid_field_name"].get<std::string>();
    managerConfig->m_crackHashRequestDatabaseConfig.m_crackHashSubRequestUuidFieldName = \
        jsonCrackHashRequestDatabaseConfig["crack_hash_sub_request_uuid_field_name"].get<std::string>();
    managerConfig->m_crackHashRequestDatabaseConfig.m_crackHashRequestHashFieldName = \
        jsonCrackHashRequestDatabaseConfig["crack_hash_request_hash_field_name"].get<std::string>();
    managerConfig->m_crackHashRequestDatabaseConfig.m_crackHashRequestMaxLengthFieldName = \
        jsonCrackHashRequestDatabaseConfig["crack_hash_request_max_length_field_name"].get<std::string>();
    managerConfig->m_crackHashRequestDatabaseConfig.m_crackHashRequestAlphabetFieldName = \
        jsonCrackHashRequestDatabaseConfig["crack_hash_request_alphabet_field_name"].get<std::string>();
    managerConfig->m_crackHashRequestDatabaseConfig.m_crackHashRequestFromFieldName = \
        jsonCrackHashRequestDatabaseConfig["crack_hash_request_from_field_name"].get<std::string>();
    managerConfig->m_crackHashRequestDatabaseConfig.m_crackHashRequestToFieldName = \
        jsonCrackHashRequestDatabaseConfig["crack_hash_request_to_field_name"].get<std::string>();
    managerConfig->m_crackHashRequestDatabaseConfig.m_crackHashRequestResultFieldName = \
        jsonCrackHashRequestDatabaseConfig["crack_hash_request_result_field_name"].get<std::string>();
    managerConfig->m_crackHashRequestDatabaseConfig.m_crackHashRequestStatusFieldName = \
        jsonCrackHashRequestDatabaseConfig["crack_hash_request_status_field_name"].get<std::string>();
}

void ManagerConfigParser::parseManagerParameters(const nlohmann::json &jsonManagerConfig) {
    auto *managerConfig = ManagerConfig::getInstance();

    managerConfig->m_startDelay = \
        jsonManagerConfig["start_delay"].get<std::uint16_t>();
    managerConfig->m_nThreads = \
        jsonManagerConfig["n_threads"].get<std::size_t>();
    managerConfig->m_maxLength = \
        jsonManagerConfig["max_length"].get<std::size_t>();
    managerConfig->m_alphabet = \
        jsonManagerConfig["alphabet"].get<std::string>();
    managerConfig->m_crackHashRequestTimeout = \
        jsonManagerConfig["crack_hash_request_timeout"].get<std::uint16_t>();
    managerConfig->m_splitInto = \
        jsonManagerConfig["split_into"].get<std::uint16_t>();
}

} // namespace config
} // namespace manager
} // namespace ch