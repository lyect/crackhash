#include "Worker.hpp"

#include "common/crack_hash_request/SubCrackHashRequest.hpp"
#include "common/crack_hash_request/CrackHashRequestIterator.hpp"
#include "common/join_stringable.hpp"
#include "common/run_delayed.hpp"

#include "manager/message/messages.hpp"

#include "worker/config/WorkerConfig.hpp"
#include "worker/config/WorkerConfigParser.hpp"

#include "tp/spdlog/sinks/stdout_color_sinks.h"
#include "tp/spdlog/sinks/basic_file_sink.h"
#include "tp/spdlog/async_logger.h"
#include "tp/spdlog/async.h"
#include "tp/spdlog/spdlog-inl.h"

#include <cryptopp/hex.h>

#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>

#include <functional>

namespace ch {
namespace worker {

Worker::Worker(
        boost::asio::io_context &ioc)
    : m_ioc{ioc}
{}

int Worker::run(const std::string &configFilePath) {
    if (!config::WorkerConfigParser::read(configFilePath)) {
        std::cerr << "Failed to parse config" << std::endl;
        return EXIT_FAILURE;
    }

    registerLogger();

    const auto *workerConfig = config::WorkerConfig::getInstance();

    std::this_thread::sleep_for(std::chrono::seconds(workerConfig->startDelay()));
    
    // Message broker connection handler
    m_messageBrokerHandler = std::make_shared<common::CallbackLibBoostAsioHandler>(
        m_ioc, std::bind(&Worker::onMessageBrokerConnectionReady, this));

    // Message broker connection
    m_messageBrokerConnection = std::make_shared<AMQP::TcpConnection>(
        m_messageBrokerHandler.get(),
        AMQP::Address(
            workerConfig->messageBrokerConfig().hostname(),
            workerConfig->messageBrokerConfig().port(),
            AMQP::Login(
                workerConfig->messageBrokerConfig().user(),
                workerConfig->messageBrokerConfig().password()),
            "/"));

    std::vector<std::thread> threads;
    threads.reserve(workerConfig->nThreads());

    for (std::size_t i = 0; i < workerConfig->nThreads(); ++i) {
        threads.push_back(std::thread{
            [this]() -> void {
                std::size_t handlersExecuted = m_ioc.run();
                m_logger->info("Executed " + std::to_string(handlersExecuted) + " handlers");
            }
        });
    }

    for (std::size_t i = 0; i < workerConfig->nThreads(); ++i) {
        threads[i].join();
    }

    return EXIT_SUCCESS;
}

void Worker::registerLogger() {
    const auto *workerConfig = config::WorkerConfig::getInstance();

    std::vector<spdlog::sink_ptr> sinks;

    // stdout sink
    if (workerConfig->loggerConfig().stdLoggerConfigPresented()) {
        auto std_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        std_sink->set_level(workerConfig->loggerConfig().stdLoggerConfig().level());
        sinks.push_back(std_sink);
    }

    // File sink
    if (workerConfig->loggerConfig().fileLoggerConfigPresented()) {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            workerConfig->loggerConfig().fileLoggerConfig().loggingFilePath(),
            workerConfig->loggerConfig().fileLoggerConfig().truncate());

        file_sink->set_level(workerConfig->loggerConfig().fileLoggerConfig().level());
        sinks.push_back(file_sink);
    }

    if (sinks.empty()) {
        // Default sink
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    }

    spdlog::init_thread_pool(
        workerConfig->loggerConfig().queueSize(),
        workerConfig->loggerConfig().nThreads());

    m_logger = std::make_shared<spdlog::async_logger>(
        workerConfig->loggerConfig().loggerName(),
        sinks.begin(), sinks.end(), spdlog::thread_pool());

    spdlog::register_logger(m_logger);

    m_logger->info("Worker logger registered successfully!");
}

void Worker::onMessageBrokerConnectionReady() {
    const auto *workerConfig = config::WorkerConfig::getInstance();

    // Message broker publish channel
    m_messageBrokerPublishChannel = \
        std::make_shared<AMQP::TcpChannel>(m_messageBrokerConnection.get());
    m_messageBrokerPublishChannel->declareExchange(
        workerConfig->messageBrokerConfig().fromWorkersExchangePoint(),
        AMQP::ExchangeType::direct);
    m_messageBrokerPublishChannel->declareQueue(
        workerConfig->messageBrokerConfig().fromWorkersQueue());
    m_messageBrokerPublishChannel->bindQueue(
        workerConfig->messageBrokerConfig().fromWorkersExchangePoint(),
        workerConfig->messageBrokerConfig().fromWorkersQueue(),
        workerConfig->messageBrokerConfig().fromWorkersRoutingKey()
    ).onSuccess([this, workerConfig]() -> void {
        m_logger->info("Created publishing channel");
        
        m_messageBrokerReliablePublishChannel = \
            std::make_shared<AMQP::Reliable<AMQP::Tagger>>(*m_messageBrokerPublishChannel.get());
        m_logger->info("Created reliable publishing channel");
    }).onError([this](const char *errorMessage) -> void {
        m_logger->error(errorMessage);
    });

    // Message broker consume channel
    m_messageBrokerConsumeChannel = \
        std::make_shared<AMQP::TcpChannel>(m_messageBrokerConnection.get());
    m_messageBrokerConsumeChannel->declareExchange(
        workerConfig->messageBrokerConfig().toWorkersExchangePoint(),
        AMQP::ExchangeType::direct);
    m_messageBrokerConsumeChannel->declareQueue(
        workerConfig->messageBrokerConfig().toWorkersQueue());
    m_messageBrokerConsumeChannel->bindQueue(
        workerConfig->messageBrokerConfig().toWorkersExchangePoint(),
        workerConfig->messageBrokerConfig().toWorkersQueue(),
        workerConfig->messageBrokerConfig().toWorkersRoutingKey()
    ).onSuccess([this, workerConfig]() -> void {
        m_logger->info("Created consuming channel");
        receiveMessageFromManager();
    }).onError([this](const char *errorMessage) -> void {
        m_logger->error(errorMessage);
    });
}

void Worker::receiveMessageFromManager() {
    const auto *workerConfig = config::WorkerConfig::getInstance();

    m_messageBrokerConsumeChannel->consume(
        workerConfig->messageBrokerConfig().toWorkersQueue()
    ).onReceived(
        std::bind(
            &Worker::successfullyReceivedMessageFromManager, this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3)
    ).onError(
        std::bind(
            &Worker::futilelyReceivedMessageFromManager, this,
            std::placeholders::_1)
    );
}

void Worker::successfullyReceivedMessageFromManager(
    const AMQP::Message &message,
    std::uint64_t deliveryTag,
    __attribute__((unused)) bool redelivered
) {
    auto jsonMessage = nlohmann::json::parse(std::string(message.body(), message.bodySize()));

    std::string strCrackHashRequestUuid        = jsonMessage["request_id"].get<std::string>();
    std::string strCrackHashSubRequestUuid     = jsonMessage["sub_request_id"].get<std::string>();
    boost::uuids::uuid crackHashRequestUuid    = boost::uuids::string_generator()(strCrackHashRequestUuid);
    boost::uuids::uuid crackHashSubRequestUuid = boost::uuids::string_generator()(strCrackHashSubRequestUuid);

    if (m_crackHashSubRequestsInWork.contains({crackHashRequestUuid, crackHashSubRequestUuid})) {
        m_logger->warn(common::joinStringable(                                                    \
            "Re-received for CrackHashSubRequest[\n",                                             \
            "\tCrackHashRequestUUID=", boost::uuids::to_string(crackHashRequestUuid), "\n",       \
            "\tCrackHashSubRequestUUID=", boost::uuids::to_string(crackHashSubRequestUuid), "\n", \
            "]"));
        m_messageBrokerConsumeChannel->ack(deliveryTag);
    }

    std::string hash      = jsonMessage["hash"].get<std::string>();
    std::size_t maxLength = jsonMessage["max_length"].get<std::size_t>();
    std::string alphabet  = jsonMessage["alphabet"].get<std::string>();
    std::string from      = jsonMessage["from"].get<std::string>();
    std::string to        = jsonMessage["to"].get<std::string>();

    auto crackHashSubRequest = common::SubCrackHashRequest::createSubCrackHashRequest(
        hash, maxLength, alphabet, from, to);

    startProcessCrackHashSubRequest(
        std::move(crackHashRequestUuid),
        std::move(crackHashSubRequestUuid),
        crackHashSubRequest,
        deliveryTag);
}

void Worker::futilelyReceivedMessageFromManager(const char *errorMessage) {
    m_logger->warn(common::joinStringable(
        "Failed to receive message from manager: ",
        errorMessage));
    
    const auto *workerConfig = config::WorkerConfig::getInstance();

    common::runDelayed(
        m_ioc,
        workerConfig->messageBrokerConfig().consumeInterval(),
        boost::bind(&Worker::receiveMessageFromManager, this));
}

void Worker::sendCrackHashSubRequestResult(
    boost::uuids::uuid crackHashRequestUuid,
    boost::uuids::uuid crackHashSubRequestUuid,
    std::vector<std::string> crackHashSubRequestResult,
    std::uint64_t crackHashSubRequestDeliveryTag
) {
    const auto *workerConfig = config::WorkerConfig::getInstance();

    auto message = manager::worker_manager_crackHashSubRequestReady(
        crackHashRequestUuid,
        crackHashSubRequestUuid,
        crackHashSubRequestResult).dump();

    AMQP::Envelope envelope(message);
    envelope.setDeliveryMode(2);

    m_messageBrokerReliablePublishChannel->publish(
        workerConfig->messageBrokerConfig().fromWorkersExchangePoint(),
        workerConfig->messageBrokerConfig().fromWorkersRoutingKey(),
        envelope
    ).onAck(
        std::bind(
            &Worker::onSendCrackHashSubRequestResultAck, this,
            crackHashRequestUuid,
            crackHashSubRequestUuid,
            crackHashSubRequestDeliveryTag)
    ).onLost(
        std::bind(
            &Worker::onSendCrackHashSubRequestResultLost, this,
            crackHashRequestUuid,
            crackHashSubRequestUuid)
    ).onError(
        std::bind(
            &Worker::onSendCrackHashSubRequestResultError, this,
            crackHashRequestUuid,
            crackHashSubRequestUuid,
            std::placeholders::_1));
}

void Worker::onSendCrackHashSubRequestResultAck(
    boost::uuids::uuid crackHashRequestUuid,
    boost::uuids::uuid crackHashSubRequestUuid,
    std::uint64_t crackHashSubRequestDeliveryTag
) {
    m_logger->info(common::joinStringable(                                                    \
        "Sent result of CrackHashSubRequest[\n",                                                 \
        "\tCrackHashRequestUUID=", boost::uuids::to_string(crackHashRequestUuid), "\n",       \
        "\tCrackHashSubRequestUUID=", boost::uuids::to_string(crackHashSubRequestUuid), "\n", \
        "]"));

    m_messageBrokerConsumeChannel->ack(crackHashSubRequestDeliveryTag);

    receiveMessageFromManager();
}

void Worker::onSendCrackHashSubRequestResultLost(
    boost::uuids::uuid crackHashRequestUuid,
    boost::uuids::uuid crackHashSubRequestUuid
) {
    m_logger->warn(common::joinStringable(                                                    \
        "Lost result of CrackHashSubRequest[\n",                                              \
        "\tCrackHashRequestUUID=", boost::uuids::to_string(crackHashRequestUuid), "\n",       \
        "\tCrackHashSubRequestUUID=", boost::uuids::to_string(crackHashSubRequestUuid), "\n", \
        "]"));
}

void Worker::onSendCrackHashSubRequestResultError(
    boost::uuids::uuid crackHashRequestUuid,
    boost::uuids::uuid crackHashSubRequestUuid,
    const char *errorMessage
) {
    m_logger->error(common::joinStringable(                                                   \
        "Failed to send result of CrackHashSubRequest[\n",                                    \
        "\tCrackHashRequestUUID=", boost::uuids::to_string(crackHashRequestUuid), "\n",       \
        "\tCrackHashSubRequestUUID=", boost::uuids::to_string(crackHashSubRequestUuid), "\n", \
        "]: ", errorMessage));
}

void Worker::startProcessCrackHashSubRequest(
    boost::uuids::uuid crackHashRequestUuid,
    boost::uuids::uuid crackHashSubRequestUuid,
    std::shared_ptr<common::CrackHashRequest> crackHashSubRequest,
    std::uint64_t deliveryTag
) {
    m_logger->info(common::joinStringable(
        "Bruteforcing from ",
        *crackHashSubRequest->begin(),
        " to ",
        *crackHashSubRequest->end()));
    boost::asio::post(
        m_ioc,
        boost::bind(
            &Worker::processCrackHashSubRequestPortion, this,
            std::move(crackHashRequestUuid),
            std::move(crackHashSubRequestUuid),
            crackHashSubRequest,
            crackHashSubRequest->begin(),
            std::vector<std::string>(),
            deliveryTag));
}

void Worker::processCrackHashSubRequestPortion(
    boost::uuids::uuid crackHashRequestUuid,
    boost::uuids::uuid crackHashSubRequestUuid,
    std::shared_ptr<common::CrackHashRequest> crackHashSubRequest,
    common::CrackHashRequestIterator currentIt,
    std::vector<std::string> crackHashSubRequestResult,
    std::uint64_t deliveryTag
) {
    if (currentIt == crackHashSubRequest->end()) {
        m_crackHashSubRequestsInWork.erase({crackHashRequestUuid, crackHashSubRequestUuid});
        m_logger->info(common::joinStringable(                                                    \
            "Finished processing CrackHashSubRequest[\n",                                         \
            "\tCrackHashRequestUUID=", boost::uuids::to_string(crackHashRequestUuid), "\n",       \
            "\tCrackHashSubRequestUUID=", boost::uuids::to_string(crackHashSubRequestUuid), "\n", \
            "]"));
        
        sendCrackHashSubRequestResult(
            std::move(crackHashRequestUuid),
            std::move(crackHashSubRequestUuid),
            std::move(crackHashSubRequestResult),
            deliveryTag);
        return;
    }

    if (currentIt == crackHashSubRequest->begin()) {
        m_crackHashSubRequestsInWork.insert({crackHashRequestUuid, crackHashSubRequestUuid});
        m_logger->info(common::joinStringable(                                                    \
            "Started processing CrackHashSubRequest[\n",                                          \
            "\tCrackHashRequestUUID=", boost::uuids::to_string(crackHashRequestUuid), "\n",       \
            "\tCrackHashSubRequestUUID=", boost::uuids::to_string(crackHashSubRequestUuid), "\n", \
            "]"));
    }

    auto *workerConfig = config::WorkerConfig::getInstance();

    for (std::size_t b = 0; b <= workerConfig->burst(); ++b) {
        if (currentIt == crackHashSubRequest->end()) {
            boost::asio::post(
                m_ioc,
                boost::bind(
                    &Worker::processCrackHashSubRequestPortion, this,
                    std::move(crackHashRequestUuid),
                    std::move(crackHashSubRequestUuid),
                    crackHashSubRequest,
                    currentIt,
                    std::move(crackHashSubRequestResult),
                    deliveryTag));
            return;
        }
        if (checkHash(*currentIt, crackHashSubRequest->hash())) {
            m_logger->info(common::joinStringable(                                                    \
                "Found \"", *currentIt, "\" for CrackHashSubRequest[\n",                              \
                "\tCrackHashRequestUUID=", boost::uuids::to_string(crackHashRequestUuid), "\n",       \
                "\tCrackHashSubRequestUUID=", boost::uuids::to_string(crackHashSubRequestUuid), "\n", \
                "]"));
        
            crackHashSubRequestResult.push_back(*currentIt);
        }
        ++currentIt;
    }

    boost::asio::post(
        m_ioc,
        boost::bind(
            &Worker::processCrackHashSubRequestPortion, this,
            std::move(crackHashRequestUuid),
            std::move(crackHashSubRequestUuid),
            crackHashSubRequest,
            currentIt,
            std::move(crackHashSubRequestResult),
            deliveryTag));
}

bool Worker::checkHash(
    const std::string &text,
    const std::string &hash
) {
    CryptoPP::byte digest[CryptoPP::Weak::MD5::DIGESTSIZE];
    m_md5hasher.CalculateDigest(digest, (const CryptoPP::byte*)text.data(), text.length());

    CryptoPP::HexEncoder encoder;
    std::string trueHash;

    encoder.Attach(new CryptoPP::StringSink(trueHash));
    encoder.Put(digest, sizeof(digest));
    encoder.MessageEnd();

    for (auto &c : trueHash) {
        if (std::isalpha(c)) {
            c = std::tolower(c);
        }
    }

    return hash == trueHash;
}

} // namespace worker
} // namespace ch