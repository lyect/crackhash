#pragma once

#include "manager/http/IncomingHttpConnection.hpp"

#include "tp/spdlog/spdlog.h"

#include <boost/beast/core/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <string>
#include <memory>

namespace ch {
namespace manager {
namespace http {

class IncomingHttpConnectionHandler {

public:

    explicit IncomingHttpConnectionHandler(boost::asio::io_context &ioc);

    bool acceptConnections(
        const std::string &ipAddressString,
        std::uint16_t ipPort,
        CreateResponseFunctorType createResponseFunctor);

private:

    std::shared_ptr<spdlog::logger> m_logger;

    boost::asio::io_context &m_ioc;
    boost::asio::ip::tcp::acceptor m_acceptor;

    CreateResponseFunctorType m_createResponseFunctor;

    void startAccept();
    void handleAccept(
        boost::beast::error_code ec,
        std::shared_ptr<IncomingHttpConnection> connection);
};

} // namespace http
} // namespace manager
} // namespace ch