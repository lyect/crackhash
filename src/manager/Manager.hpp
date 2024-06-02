#pragma once

#include "common/broker/CallbackLibBoostAsioHandler.hpp"
#include "manager/http/IncomingHttpConnectionHandler.hpp"
#include "manager/db/CrackHashRequestsDbHandler.hpp"

#include "tp/spdlog/logger.h"

#include <boost/asio/io_context.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <amqpcpp.h>
#include <amqpcpp/linux_tcp.h>

#include <vector>
#include <memory>

namespace ch {
namespace manager {

class Manager : public http::IncomingHttpConnectionHandler {

public:

    explicit Manager(boost::asio::io_context &ioc);

    int run(const std::string &configFilePath);

private:

    std::shared_ptr<spdlog::logger> m_logger;

    boost::asio::io_context &m_ioc;
    
    std::shared_ptr<common::CallbackLibBoostAsioHandler> m_messageBrokerHandler;
    std::shared_ptr<AMQP::TcpConnection>                 m_messageBrokerConnection;
    std::shared_ptr<AMQP::TcpChannel>                    m_messageBrokerConsumeChannel;
    std::shared_ptr<AMQP::TcpChannel>                    m_messageBrokerPublishChannel;
    std::shared_ptr<AMQP::Reliable<AMQP::Tagger>>        m_messageBrokerReliablePublishChannel;

    std::shared_ptr<CrackHashRequestsDbHandler> m_dbHandler;

    boost::uuids::random_generator m_uuidGenerator;

    void registerLogger();

    void onMessageBrokerConnectionReady();
    void onMessageBrokerConsumeChannelGetSuccess();

    http::ResponseType createResponse(const http::RequestType &request);
    http::ResponseType processPostRequest(const http::RequestType &request);
    bool checkPostRequestParameters(
        const http::RequestType  &request,
              http::ResponseType &response,
              std::string        &hash,
              std::uint64_t      &maxLength);
    http::ResponseType processGetRequest(const http::RequestType &request);
    bool checkGetRequestParameters(
        const http::RequestType  &request,
              http::ResponseType &response,
              boost::uuids::uuid &crackHashRequestUuid);

    void sendCrackHashSubRequest(
        boost::uuids::uuid crackHashRequestUuid,
        boost::uuids::uuid crackHashSubRequestUuid,
        std::shared_ptr<common::CrackHashRequest> crackHashSubRequest);
    void onSendCrackHashSubRequestError(
        boost::uuids::uuid crackHashRequestUuid,
        boost::uuids::uuid crackHashSubRequestUuid,
        const char *errorMessage);
    void onSendCrackHashSubRequestAck(
        boost::uuids::uuid crackHashRequestUuid,
        boost::uuids::uuid crackHashSubRequestUuid);
    void onSendCrackHashSubRequestLost(
        boost::uuids::uuid crackHashRequestUuid,
        boost::uuids::uuid crackHashSubRequestUuid);
    
    void receiveMessageFromWorker();
    void successfullyReceivedMessageFromWorker(
        const AMQP::Message &message,
        std::uint64_t deliveryTag,
        bool redelivered);
    void futilelyReceivedMessageFromWorker(
        const char *errorMessage);

    void checkCrackHashSubRequestTimeout(
        boost::uuids::uuid crackHashRequestUuid,
        boost::uuids::uuid crackHashSubRequestUuid);
};

} // namespace manager
} // namespace ch