#pragma once

#include "common/broker/CallbackLibBoostAsioHandler.hpp"
#include "common/crack_hash_request/CrackHashRequest.hpp"

#include "tp/spdlog/spdlog.h"
#include "tp/nlohmann/json.hpp"

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/asio/io_context.hpp>

#include <amqpcpp.h>
#include <amqpcpp/linux_tcp.h>

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/md5.h>

#include <memory>
#include <unordered_set>

namespace ch {
namespace worker {

class Worker {

public:

    explicit Worker(boost::asio::io_context &ioc);

    int run(const std::string &configFilePath);

private:
    std::shared_ptr<spdlog::logger> m_logger;

    boost::asio::io_context &m_ioc;

    std::shared_ptr<common::CallbackLibBoostAsioHandler> m_messageBrokerHandler;
    std::shared_ptr<AMQP::TcpConnection>                 m_messageBrokerConnection;
    std::shared_ptr<AMQP::TcpChannel>                    m_messageBrokerConsumeChannel;
    std::shared_ptr<AMQP::TcpChannel>                    m_messageBrokerPublishChannel;
    std::shared_ptr<AMQP::Reliable<AMQP::Tagger>>        m_messageBrokerReliablePublishChannel;

    CryptoPP::Weak::MD5 m_md5hasher;

    std::unordered_set<
        std::pair<
            boost::uuids::uuid,
            boost::uuids::uuid>,
        boost::hash<
            std::pair<
                boost::uuids::uuid,
                boost::uuids::uuid
    >>> m_crackHashSubRequestsInWork;

    void registerLogger();

    void onMessageBrokerConnectionReady();

    void receiveMessageFromManager();
    void successfullyReceivedMessageFromManager(
        const AMQP::Message &message,
        std::uint64_t deliveryTag,
        bool redelivered);
    void futilelyReceivedMessageFromManager(
        const char *errorMessage);

    void sendCrackHashSubRequestResult(
        boost::uuids::uuid crackHashRequestUuid,
        boost::uuids::uuid crackHashSubRequestUuid,
        std::vector<std::string> crackHashSubRequestResult,
        std::uint64_t crackHashSubRequestDeliveryTag);
    void onSendCrackHashSubRequestResultAck(
        boost::uuids::uuid crackHashRequestUuid,
        boost::uuids::uuid crackHashSubRequestUuid,
        std::uint64_t crackHashSubRequestDeliveryTag);
    void onSendCrackHashSubRequestResultLost(
        boost::uuids::uuid crackHashRequestUuid,
        boost::uuids::uuid crackHashSubRequestUuid);
    void onSendCrackHashSubRequestResultError(
        boost::uuids::uuid crackHashRequestUuid,
        boost::uuids::uuid crackHashSubRequestUuid,
        const char *errorMessage);

    void startProcessCrackHashSubRequest(
        boost::uuids::uuid crackHashRequestUuid,
        boost::uuids::uuid crackHashSubRequestUuid,
        std::shared_ptr<common::CrackHashRequest> crackHashSubRequest,
        std::uint64_t deliveryTag);
    void processCrackHashSubRequestPortion(
        boost::uuids::uuid crackHashRequestUuid,
        boost::uuids::uuid crackHashSubRequestUuid,
        std::shared_ptr<common::CrackHashRequest> crackHashSubRequest,
        common::CrackHashRequestIterator currentIt,
        std::vector<std::string> result,
        std::uint64_t deliveryTag);
    
    bool checkHash(
        const std::string &text,
        const std::string &hash);
    
};

} // namespace worker
} // namespace ch