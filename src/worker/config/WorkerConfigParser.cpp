#include "WorkerConfigParser.hpp"

#include "common/config/utilities.hpp"

#include "worker/config/WorkerConfig.hpp"

#include <iostream>
#include <fstream>
#include <cstring>
#include <cerrno>

namespace ch {
namespace worker {
namespace config {

bool WorkerConfigParser::read(const std::string &configFilePath) {

    std::ifstream ifs(configFilePath);

    if (!ifs.is_open()) {
        std::cerr << "Failed to open config file: " << strerror(errno) << std::endl;
        return false;
    }

    WorkerConfig *workerConfig = new WorkerConfig();
    common::config::CommonConfig::reset(workerConfig);

    nlohmann::json jsonWorkerConfig;
    try {
        jsonWorkerConfig = nlohmann::json::parse(ifs);
        parseLoggerConfig(jsonWorkerConfig);
        parseMessageBrokerConfig(jsonWorkerConfig);
        parseWorkerParameters(jsonWorkerConfig);
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        ifs.close();
        return false;
    }

    ifs.close();
    return true;
}

void WorkerConfigParser::parseLoggerConfig(const nlohmann::json &jsonWorkerConfig) {
    auto *workerConfig = WorkerConfig::getInstance();

    const auto &jsonWorkerLoggerConfig = jsonWorkerConfig["logger"];

    workerConfig->m_loggerConfig.m_loggerName = \
        jsonWorkerLoggerConfig["name"].get<std::string>();
    workerConfig->m_loggerConfig.m_queueSize = \
        jsonWorkerLoggerConfig["queue_size"].get<std::size_t>();
    workerConfig->m_loggerConfig.m_nThreads = \
        jsonWorkerLoggerConfig["n_threads"].get<std::size_t>();

    if (jsonWorkerLoggerConfig.contains("file_logger")) {
        auto &jsonFileLoggerConfig = jsonWorkerLoggerConfig["file_logger"];

        workerConfig->m_loggerConfig.m_fileLoggerConfig = new common::config::FileLoggerConfig();
        workerConfig->m_loggerConfig.m_fileLoggerConfig->m_loggingFilePath = \
            jsonFileLoggerConfig["path"].get<std::string>();
        workerConfig->m_loggerConfig.m_fileLoggerConfig->m_truncate = \
            jsonFileLoggerConfig["truncate"].get<bool>();
        workerConfig->m_loggerConfig.m_fileLoggerConfig->m_level = \
            common::fileLoggerLevelFromStringToEnum(jsonFileLoggerConfig["level"].get<std::string>()); 
    }

    if (jsonWorkerLoggerConfig.contains("std_logger")) {
        auto &jsonStdLoggerConfig = jsonWorkerLoggerConfig["std_logger"];

        workerConfig->m_loggerConfig.m_stdLoggerConfig = new common::config::StdLoggerConfig();
        workerConfig->m_loggerConfig.m_stdLoggerConfig->m_level = \
           common::stdLoggerLevelFromStringToEnum(jsonStdLoggerConfig["level"].get<std::string>()); 
    }
}

void WorkerConfigParser::parseMessageBrokerConfig(const nlohmann::json &jsonWorkerConfig) {
    auto *workerConfig = WorkerConfig::getInstance();

    const auto &jsonMessageBrokerConfig = jsonWorkerConfig["message_broker"];
    workerConfig->m_messageBrokerConfig.m_hostname = \
        jsonMessageBrokerConfig["hostname"].get<std::string>();
    workerConfig->m_messageBrokerConfig.m_port = \
        jsonMessageBrokerConfig["port"].get<std::uint16_t>();
    workerConfig->m_messageBrokerConfig.m_user = \
        jsonMessageBrokerConfig["user"].get<std::string>();
    workerConfig->m_messageBrokerConfig.m_password = \
        jsonMessageBrokerConfig["password"].get<std::string>();
    workerConfig->m_messageBrokerConfig.m_toWorkersQueue = \
        jsonMessageBrokerConfig["to_workers_queue"].get<std::string>();
    workerConfig->m_messageBrokerConfig.m_toWorkersRoutingKey = \
        jsonMessageBrokerConfig["to_workers_routing_key"].get<std::string>();
    workerConfig->m_messageBrokerConfig.m_toWorkersExchangePoint = \
        jsonMessageBrokerConfig["to_workers_exchange_point"].get<std::string>();
    workerConfig->m_messageBrokerConfig.m_fromWorkersQueue = \
        jsonMessageBrokerConfig["from_workers_queue"].get<std::string>();
    workerConfig->m_messageBrokerConfig.m_fromWorkersRoutingKey = \
        jsonMessageBrokerConfig["from_workers_routing_key"].get<std::string>();
    workerConfig->m_messageBrokerConfig.m_fromWorkersExchangePoint = \
        jsonMessageBrokerConfig["from_workers_exchange_point"].get<std::string>();
    workerConfig->m_messageBrokerConfig.m_publishInterval = \
        jsonMessageBrokerConfig["publish_interval"].get<std::uint16_t>();
    workerConfig->m_messageBrokerConfig.m_consumeInterval = \
        jsonMessageBrokerConfig["consume_interval"].get<std::uint16_t>();
}

void WorkerConfigParser::parseWorkerParameters(const nlohmann::json &jsonWorkerConfig) {
    auto *workerConfig = WorkerConfig::getInstance();

    workerConfig->m_startDelay = \
        jsonWorkerConfig["start_delay"].get<std::uint16_t>();
    workerConfig->m_nThreads = \
        jsonWorkerConfig["n_threads"].get<std::size_t>();
    workerConfig->m_burst = \
        jsonWorkerConfig["burst"].get<std::size_t>();
}

} // namespace config
} // namespace worker
} // namespace ch