#include "Manager.hpp"

#include "client/message/messages.hpp"

#include "common/crack_hash_request/FullCrackHashRequest.hpp"
#include "common/crack_hash_request/CrackHashRequestStatus.hpp"
#include "common/join_stringable.hpp"
#include "common/run_delayed.hpp"

#include "manager/config/ManagerConfig.hpp"
#include "manager/config/ManagerConfigParser.hpp"
#include "manager/http/utilities.hpp"

#include "worker/message/messages.hpp"

#include "tp/spdlog/sinks/stdout_color_sinks.h"
#include "tp/spdlog/sinks/basic_file_sink.h"
#include "tp/spdlog/async_logger.h"
#include "tp/spdlog/async.h"
#include "tp/spdlog/spdlog-inl.h"

#include <boost/uuid/uuid_io.hpp>

#include <iostream>

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

namespace ch {
namespace manager {

Manager::Manager(boost::asio::io_context &ioc)
    : http::IncomingHttpConnectionHandler(ioc)
    , m_ioc{ioc}
{}

int Manager::run(const std::string &configFilePath) {
    if (!config::ManagerConfigParser::read(configFilePath)) {
        std::cerr << "Failed to parse config" << std::endl;
        return EXIT_FAILURE;
    }

    registerLogger();

    const auto *managerConfig = config::ManagerConfig::getInstance();

    std::this_thread::sleep_for(std::chrono::seconds(managerConfig->startDelay()));

    m_dbHandler = std::make_shared<CrackHashRequestsDbHandler>();
    
    // Message broker connection handler
    m_messageBrokerHandler = std::make_shared<common::CallbackLibBoostAsioHandler>(
        m_ioc, std::bind(&Manager::onMessageBrokerConnectionReady, this));

    // Message broker connection
    m_messageBrokerConnection = std::make_shared<AMQP::TcpConnection>(
        m_messageBrokerHandler.get(),
        AMQP::Address(
            managerConfig->messageBrokerConfig().hostname(),
            managerConfig->messageBrokerConfig().port(),
            AMQP::Login(
                managerConfig->messageBrokerConfig().user(),
                managerConfig->messageBrokerConfig().password()),
            "/"));

    std::vector<std::thread> threads;
    threads.reserve(managerConfig->nThreads());

    for (std::size_t i = 0; i < managerConfig->nThreads(); ++i) {
        threads.push_back(std::thread{
            [this]() -> void {
                std::size_t handlersExecuted = m_ioc.run();
                m_logger->info("Executed " + std::to_string(handlersExecuted) + " handlers");
            }
        });
    }

    for (std::size_t i = 0; i < managerConfig->nThreads(); ++i) {
        threads[i].join();
    }

    return EXIT_SUCCESS;
}

void Manager::registerLogger() {
    const auto *managerConfig = config::ManagerConfig::getInstance();

    std::vector<spdlog::sink_ptr> sinks;

    // stdout sink
    if (managerConfig->loggerConfig().stdLoggerConfigPresented()) {
        auto std_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        std_sink->set_level(managerConfig->loggerConfig().stdLoggerConfig().level());
        sinks.push_back(std_sink);
    }

    // File sink
    if (managerConfig->loggerConfig().fileLoggerConfigPresented()) {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            managerConfig->loggerConfig().fileLoggerConfig().loggingFilePath(),
            managerConfig->loggerConfig().fileLoggerConfig().truncate());

        file_sink->set_level(managerConfig->loggerConfig().fileLoggerConfig().level());
        sinks.push_back(file_sink);
    }

    if (sinks.empty()) {
        // Default sink
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    }

    spdlog::init_thread_pool(
        managerConfig->loggerConfig().queueSize(),
        managerConfig->loggerConfig().nThreads());

    m_logger = std::make_shared<spdlog::async_logger>(
        managerConfig->loggerConfig().loggerName(),
        sinks.begin(), sinks.end(), spdlog::thread_pool());

    spdlog::register_logger(m_logger);

    m_logger->info("Manager logger registered successfully!");
}

void Manager::onMessageBrokerConnectionReady() {
    const auto *managerConfig = config::ManagerConfig::getInstance();

    // Message broker publish channel
    m_messageBrokerPublishChannel = \
        std::make_shared<AMQP::TcpChannel>(m_messageBrokerConnection.get());
    m_messageBrokerPublishChannel->declareExchange(
        managerConfig->messageBrokerConfig().toWorkersExchangePoint(),
        AMQP::ExchangeType::direct);
    m_messageBrokerPublishChannel->declareQueue(
        managerConfig->messageBrokerConfig().toWorkersQueue());
    m_messageBrokerPublishChannel->bindQueue(
        managerConfig->messageBrokerConfig().toWorkersExchangePoint(),
        managerConfig->messageBrokerConfig().toWorkersQueue(),
        managerConfig->messageBrokerConfig().toWorkersRoutingKey()
    ).onSuccess([this, managerConfig]() -> void {
        m_logger->info("Created publishing channel");
        m_messageBrokerReliablePublishChannel = \
            std::make_shared<AMQP::Reliable<AMQP::Tagger>>(*m_messageBrokerPublishChannel.get());
        m_logger->info("Created reliable publishing channel");
        acceptConnections(
            managerConfig->incomingHttpConnectionConfig().ipAddress(),
            managerConfig->incomingHttpConnectionConfig().ipPort(),
            boost::bind(
                &Manager::createResponse, this,
                boost::placeholders::_1));
    }).onError([this](const char *errorMessage) -> void {
        m_logger->error(common::joinStringable(
            "Failed to open publish channel within connection:",
            errorMessage));
    });

    // Message broker consume channel
    m_messageBrokerConsumeChannel = \
        std::make_shared<AMQP::TcpChannel>(m_messageBrokerConnection.get());
    m_messageBrokerConsumeChannel->declareExchange(
        managerConfig->messageBrokerConfig().fromWorkersExchangePoint(),
        AMQP::ExchangeType::direct);
    m_messageBrokerConsumeChannel->declareQueue(
        managerConfig->messageBrokerConfig().fromWorkersQueue());
    m_messageBrokerConsumeChannel->bindQueue(
        managerConfig->messageBrokerConfig().fromWorkersExchangePoint(),
        managerConfig->messageBrokerConfig().fromWorkersQueue(),
        managerConfig->messageBrokerConfig().fromWorkersRoutingKey()
    ).onSuccess([this]() -> void {
        m_logger->info("Created consuming channel");
        receiveMessageFromWorker();
    }).onError([this](const char *errorMessage) -> void {
        m_logger->error(common::joinStringable(
            "Failed to open consume channel within connection:",
            errorMessage));
    });
}

http::ResponseType Manager::createResponse(const http::RequestType &request) {
    switch (request.method()) {
        case boost::beast::http::verb::post:  // only from CLIENT: crack hash
            return processPostRequest(request);
        case boost::beast::http::verb::get:   // only from CLIENT: check status
            return processGetRequest(request);
        default: {
            std::string methodStr = boost::beast::http::to_string(request.method());
            m_logger->warn("\"" + methodStr + "\" request is not allowed");

            http::ResponseType response;
            response.version(request.version());
            response.keep_alive(false);
            response.result(boost::beast::http::status::bad_request);
            response.set(boost::beast::http::field::content_type, "application/json");

            nlohmann::json jsonBody = {
                {"response", "Invalid method '" + methodStr + "'"}
            };

            response.body() = jsonBody.dump();
            response.result(boost::beast::http::status::bad_request);

            return response;
        }
    }
}

http::ResponseType Manager::processPostRequest(const http::RequestType &request) {
    auto response = http::makeEmptyResponse(request);

    std::string   hash;
    std::uint64_t maxLength;

    if (!checkPostRequestParameters(request, response, hash, maxLength)) {
        m_logger->info("\"POST\" request is invalid");
        return response;
    }

    m_logger->info("\"POST\" request is valid");

    auto *managerConfig = config::ManagerConfig::getInstance();

    auto crackHashRequest = common::FullCrackHashRequest::createFullCrackHashRequest(
        hash,
        maxLength,
        managerConfig->alphabet());
    auto crackHashRequestUuid = m_uuidGenerator();

    auto crackHashSubRequests = crackHashRequest->split(managerConfig->splitInto());
    std::vector<boost::uuids::uuid> crackHashSubRequestUuids(crackHashSubRequests.size());

    // Generate sub-request UUIDs
    for (auto &crackHashSubRequestUuid: crackHashSubRequestUuids) {
        crackHashSubRequestUuid = m_uuidGenerator();
    }

    // Store sub-requests in DB
    m_dbHandler->startTransaction();
    for (
        std::size_t crackHashSubRequestIndex = 0;
        crackHashSubRequestIndex < crackHashSubRequests.size();
        ++crackHashSubRequestIndex
    ) {
        auto crackHashSubRequestUuid = crackHashSubRequestUuids[crackHashSubRequestIndex];
        auto crackHashSubRequest = crackHashSubRequests[crackHashSubRequestIndex];
        m_dbHandler->storeCrackHashSubRequest(
            crackHashRequestUuid,
            crackHashSubRequestUuid,
            crackHashSubRequest);
    }
    m_dbHandler->commitTransaction();

    // Send sub-requests to broker
    for (
        std::size_t crackHashSubRequestIndex = 0;
        crackHashSubRequestIndex < crackHashSubRequests.size();
        ++crackHashSubRequestIndex
    ) {
        auto crackHashSubRequestUuid = crackHashSubRequestUuids[crackHashSubRequestIndex];
        auto crackHashSubRequest = crackHashSubRequests[crackHashSubRequestIndex];
        
        sendCrackHashSubRequest(
            crackHashRequestUuid,
            crackHashSubRequestUuid,
            crackHashSubRequest);

        common::runDelayed(
            m_ioc,
            managerConfig->crackHashRequestTimeout(),
            boost::bind(
                &Manager::checkCrackHashSubRequestTimeout, this,
                crackHashRequestUuid,
                crackHashSubRequestUuid));
    }

    response.body() = client::manager_client_addedCrackHashRequest(
        crackHashRequestUuid
    ).dump();
    response.result(boost::beast::http::status::ok);
    return response;
}

bool Manager::checkPostRequestParameters(
    const http::RequestType  &request,
          http::ResponseType &response,
          std::string        &hash,
          std::uint64_t      &maxLength
) {
    if (request.target() != "/api/hash/crack") {
        http::makeMethodNotAllowedResponse(response, "POST", request.target());
        return false;
    }

    nlohmann::json jsonRequestBody;
    try {
        jsonRequestBody = nlohmann::json::parse(request.body());
    }
    catch (std::exception &e) {
        http::makeInvalidJsonResponse(response, e.what());
        return false;
    }

    if (!jsonRequestBody.contains("hash")) {
        http::makeInvalidJsonResponse(response, "'hash' field is not represented");
        return false;
    }

    if (!jsonRequestBody.contains("max_length")) {
        http::makeInvalidJsonResponse(response, "'max_length' field is not represented");
        return false;
    }

    // Only "hash" and "max_length" must be represented
    if (jsonRequestBody.size() > 2) {
        http::makeInvalidJsonResponse(response, "Too much fields in JSON");
        return false;
    }

    if (!jsonRequestBody["hash"].is_string()) {
        http::makeInvalidJsonResponse(response, "'hash' field must be string");
        return false;
    }

    hash = jsonRequestBody["hash"].get<std::string>();

    for (auto c : hash) {
        if (('0' <= c && c <= '9') || ('a' <= c && c <= 'f')) {
            continue;
        }

        http::makeInvalidJsonResponse(response, "'hash' must contain only HEX symbols");
        return false;
    }

    if (!jsonRequestBody["max_length"].is_number_unsigned()) {
        http::makeInvalidJsonResponse(response, "'max_length' field must be unsigned");
        return false;
    }

    maxLength = jsonRequestBody["max_length"].get<std::size_t>();

    const auto *managerConfig = config::ManagerConfig::getInstance();

    if (maxLength > managerConfig->maxLength()) {
        http::makeInvalidJsonResponse(response, "'max_length' is too large");
        return false;
    }

    return true;
}

http::ResponseType Manager::processGetRequest(const http::RequestType &request) {
    auto response = http::makeEmptyResponse(request);

    boost::uuids::uuid crackHashRequestUuid;

    if (!checkGetRequestParameters(request, response, crackHashRequestUuid)) {
        m_logger->info("\"GET\" request is invalid");
        return response;
    }

    m_logger->info("\"GET\" request is valid");

    bool found;

    auto crackHashRequestStatus = m_dbHandler->getCrackHashRequestStatus(crackHashRequestUuid, found);
    if (!found) {
        http::makeServerError(response, common::joinStringable(
            "Failed to get status of ", boost::uuids::to_string(crackHashRequestUuid)
        ));
        return response;
    } 

    std::vector<std::string> crackHashRequestResult;
    if (crackHashRequestStatus == common::CrackHashRequestStatus::READY) {
        LOG_CRACK_HASH_REQUEST_INFO("Ready");
        crackHashRequestResult = m_dbHandler->getCrackHashRequestResult(crackHashRequestUuid, found);
        LOG_CRACK_HASH_REQUEST_INFO("Got result of");
        if (!found) {
            http::makeServerError(response, common::joinStringable(
                "Failed to get result of ", boost::uuids::to_string(crackHashRequestUuid)
            ));
            return response;
        } 
    }

    response.body() = client::manager_client_crackHashRequestStatus(
        crackHashRequestUuid,
        crackHashRequestStatus,
        crackHashRequestResult
    ).dump();
    response.result(boost::beast::http::status::ok);

    return response;
}

bool Manager::checkGetRequestParameters(
    const http::RequestType  &request,
          http::ResponseType &response,
          boost::uuids::uuid &crackHashRequestUuid
) {
    auto questionPos = request.target().find("?");

    auto target = std::string(request.target().begin(), questionPos);
    if (target != "/api/hash/status") {
        http::makeMethodNotAllowedResponse(response, "GET", target);
        return false;
    }

    if (questionPos == request.target().size()) {
        http::makeInvalidParameters(response, "parameters are not specified");
        return false;
    }

    auto equalsPos = request.target().find("=", questionPos + 1);
    
    if (std::string_view(&request.target()[questionPos + 1], equalsPos - questionPos - 1) != "request_id") {
        http::makeInvalidParameters(response, "Request's UUID is not specified");
        return false;
    }
    
    if (request.target().size() - equalsPos - 1 > 36) { // UUID representation length
        http::makeInvalidParameters(response, "Only request's UUID must be specified");
        return false;
    }

    auto strCrackHashRequestUuid = \
        std::string(&request.target()[equalsPos + 1], request.target().size() - equalsPos - 1);

    bool crackHashRequestUuidIsValid;
    try {
        crackHashRequestUuid = boost::uuids::string_generator()(strCrackHashRequestUuid);
        crackHashRequestUuidIsValid = (
            crackHashRequestUuid.version() !=
            boost::uuids::uuid::version_unknown);
    } catch (...) {
        m_logger->warn("Unparseable UUID: " + strCrackHashRequestUuid);
        crackHashRequestUuidIsValid = false;
    }

    if (!crackHashRequestUuidIsValid) {
        http::makeInvalidParameters(response, "Invalid request's UUID");
        return false;
    }

    return true;
}

void Manager::sendCrackHashSubRequest(
    boost::uuids::uuid crackHashRequestUuid,
    boost::uuids::uuid crackHashSubRequestUuid,
    std::shared_ptr<common::CrackHashRequest> crackHashSubRequest
) { 
    const auto *managerConfig = config::ManagerConfig::getInstance();

    auto message = worker::manager_worker_addCrackHashSubRequest(
        crackHashRequestUuid,
        crackHashSubRequestUuid,
        crackHashSubRequest).dump();
    AMQP::Envelope envelope(message);
    envelope.setDeliveryMode(2);

    m_messageBrokerReliablePublishChannel->publish(
        managerConfig->messageBrokerConfig().toWorkersExchangePoint(),
        managerConfig->messageBrokerConfig().toWorkersRoutingKey(),
        envelope
    ).onAck(
        std::bind(
            &Manager::onSendCrackHashSubRequestAck, this,
            crackHashRequestUuid,
            crackHashSubRequestUuid)
    ).onLost(
        std::bind(
            &Manager::onSendCrackHashSubRequestLost, this,
            crackHashRequestUuid,
            crackHashSubRequestUuid)
    ).onError(
        std::bind(
            &Manager::onSendCrackHashSubRequestError, this,
            crackHashRequestUuid,
            crackHashSubRequestUuid,
            std::placeholders::_1));
}

void Manager::onSendCrackHashSubRequestError(
    boost::uuids::uuid crackHashRequestUuid,
    boost::uuids::uuid crackHashSubRequestUuid,
    const char *errorMessage
) {
    m_logger->warn(common::joinStringable(                                                    \
        "Error while sending CrackHashSubRequest[\n",                                         \
        "\tCrackHashRequestUUID=", boost::uuids::to_string(crackHashRequestUuid), "\n",       \
        "\tCrackHashSubRequestUUID=", boost::uuids::to_string(crackHashSubRequestUuid), "\n", \
        "]:",
        errorMessage));
}

void Manager::onSendCrackHashSubRequestAck(
    boost::uuids::uuid crackHashRequestUuid,
    boost::uuids::uuid crackHashSubRequestUuid
) {
    LOG_CRACK_HASH_SUB_REQUEST_INFO("Sent to broker");
}

void Manager::onSendCrackHashSubRequestLost(
    boost::uuids::uuid crackHashRequestUuid,
    boost::uuids::uuid crackHashSubRequestUuid
) {
    bool found;

    auto crackHashSubRequestStatus = m_dbHandler->getCrackHashSubRequestStatus(
        crackHashRequestUuid,
        crackHashSubRequestUuid,
        found);

    if (!found) {
        LOG_CRACK_HASH_SUB_REQUEST_WARN("Failed to process lost");
        return;
    } 

    if (crackHashSubRequestStatus == common::CrackHashRequestStatus::IN_PROGRESS) {
        LOG_CRACK_HASH_SUB_REQUEST_INFO("Lost");
        
        auto crackHashSubRequest = m_dbHandler->getCrackHashSubRequest(
            crackHashRequestUuid,
            crackHashSubRequestUuid,
            found);
        
        if (!found) {
            LOG_CRACK_HASH_SUB_REQUEST_WARN("Failed to process lost");
        }

        auto *managerConfig = config::ManagerConfig::getInstance();

        common::runDelayed(
            m_ioc,
            managerConfig->messageBrokerConfig().publishInterval(),
            boost::bind(
                &Manager::sendCrackHashSubRequest, this,
                crackHashRequestUuid,
                crackHashSubRequestUuid,
                crackHashSubRequest));
    }
}

void Manager::receiveMessageFromWorker() {
    const auto *managerConfig = config::ManagerConfig::getInstance();

    m_messageBrokerConsumeChannel->consume(
        managerConfig->messageBrokerConfig().fromWorkersQueue()
    ).onReceived(
        std::bind(
            &Manager::successfullyReceivedMessageFromWorker, this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3)
    ).onError(
        std::bind(
            &Manager::futilelyReceivedMessageFromWorker, this,
            std::placeholders::_1)
    );
}

void Manager::successfullyReceivedMessageFromWorker(
    const AMQP::Message &message,
    std::uint64_t deliveryTag,
    __attribute__((unused)) bool redelivered
) {
    auto jsonMessage = nlohmann::json::parse(std::string(message.body(), message.bodySize()));
    
    std::string strCrackHashRequestUuid    = jsonMessage["request_id"].get<std::string>();
    std::string strCrackHashSubRequestUuid = jsonMessage["sub_request_id"].get<std::string>();

    boost::uuids::uuid crackHashRequestUuid    = boost::uuids::string_generator()(strCrackHashRequestUuid);
    boost::uuids::uuid crackHashSubRequestUuid = boost::uuids::string_generator()(strCrackHashSubRequestUuid);

    std::vector<std::string> result = jsonMessage["result"];

    bool found;

    m_dbHandler->startTransaction();
    auto crackHashSubRequestStatus = m_dbHandler->getCrackHashSubRequestStatus(
        crackHashRequestUuid,
        crackHashSubRequestUuid,
        found);
    if (!found) {
        LOG_CRACK_HASH_SUB_REQUEST_WARN("Failed to process from worker");
        m_dbHandler->commitTransaction();
        return;
    }

    if (crackHashSubRequestStatus == common::CrackHashRequestStatus::IN_PROGRESS) {
        if (!m_dbHandler->setCrackHashSubRequestResult(
            crackHashRequestUuid,
            crackHashSubRequestUuid,
            std::move(result))
        ) {
            LOG_CRACK_HASH_SUB_REQUEST_WARN("Failed to process from worker");
            m_dbHandler->commitTransaction();
            return;
        }
    }

    m_dbHandler->commitTransaction();

    m_messageBrokerConsumeChannel->ack(deliveryTag);

    LOG_CRACK_HASH_SUB_REQUEST_INFO("Processed from worker");
}

void Manager::futilelyReceivedMessageFromWorker(const char *errorMessage) {
    m_logger->warn(common::joinStringable(
        "Failed to receive message from worker: ",
        errorMessage));
    
    const auto *managerConfig = config::ManagerConfig::getInstance();

    common::runDelayed(
        m_ioc,
        managerConfig->messageBrokerConfig().consumeInterval(),
        boost::bind(&Manager::receiveMessageFromWorker, this));
}

void Manager::checkCrackHashSubRequestTimeout(
    boost::uuids::uuid crackHashRequestUuid,
    boost::uuids::uuid crackHashSubRequestUuid
) {
    m_dbHandler->startTransaction();

    bool found;

    auto crackHashRequestStatus = m_dbHandler->getCrackHashSubRequestStatus(
        crackHashRequestUuid,
        crackHashSubRequestUuid,
        found);
    if (!found) {
        LOG_CRACK_HASH_SUB_REQUEST_WARN("Failed to process timed out");
        m_dbHandler->commitTransaction();
        return;
    }

    if (crackHashRequestStatus == common::CrackHashRequestStatus::IN_PROGRESS) {
        if (!m_dbHandler->setCrackHashSubRequestStatus(
            crackHashRequestUuid,
            crackHashSubRequestUuid,
            common::CrackHashRequestStatus::TIMEOUT)
        ){
            LOG_CRACK_HASH_SUB_REQUEST_WARN("Failed to process timed out");
            m_dbHandler->commitTransaction();
            return;
        }
    }

    LOG_CRACK_HASH_SUB_REQUEST_INFO("Processed timed out");
    m_dbHandler->commitTransaction();
}

} // namespace manager
} // namespace ch