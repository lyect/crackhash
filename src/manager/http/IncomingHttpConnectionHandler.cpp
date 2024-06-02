#include "IncomingHttpConnectionHandler.hpp"

#include "manager/config/ManagerConfig.hpp"

#include <boost/asio/strand.hpp>
#include <boost/bind/bind.hpp>

namespace ch {
namespace manager {
namespace http {

IncomingHttpConnectionHandler::IncomingHttpConnectionHandler(
    boost::asio::io_context &ioc)
        : m_ioc{ioc}
        , m_acceptor{boost::asio::make_strand(m_ioc)}
{}

bool IncomingHttpConnectionHandler::acceptConnections(
    const std::string &ipAddressString,
    std::uint16_t ipPort,
    CreateResponseFunctorType createResponseFunctor
) {
    m_logger = spdlog::get(config::ManagerConfig::getInstance()->loggerConfig().loggerName());
    m_createResponseFunctor = std::move(createResponseFunctor);

    boost::beast::error_code ec;

    auto const ipAddress = boost::asio::ip::make_address(ipAddressString, ec);
    if (ec) {
        m_logger->error("\"make_address\" failed: " + ec.message());
        return false;
    }

    boost::asio::ip::tcp::endpoint endpoint(ipAddress, ipPort);

    // Open the acceptor
    m_acceptor.open(endpoint.protocol(), ec);
    if (ec) {
        m_logger->error("\"open\" failed: " + ec.message());
        return false;
    }

    // Allow address reuse
    m_acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if (ec) {
        m_logger->error("\"set_option\" failed: " + ec.message());
        return false;
    }

    // Bind to the server address
    m_acceptor.bind(endpoint, ec);
    if (ec) {
        m_logger->error("\"bind\" failed: " + ec.message());
        return false;
    }

    // Start listening for connections
    m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec) {
        m_logger->error("\"listen\" failed: " + ec.message());
        return false;
    }

    startAccept();

    m_logger->info("Started accepting connections on " + endpoint.address().to_string() + ":" + std::to_string(endpoint.port()));
    return true;
}

void IncomingHttpConnectionHandler::startAccept() {
    auto connection = std::make_shared<IncomingHttpConnection>(
        m_ioc,
        m_createResponseFunctor);
    
    m_acceptor.async_accept(
        connection->socket(),
        boost::bind(
            &IncomingHttpConnectionHandler::handleAccept, this,
            boost::placeholders::_1,
            connection));
}

void IncomingHttpConnectionHandler::handleAccept(
    boost::beast::error_code ec,
    std::shared_ptr<IncomingHttpConnection> connection
) {
    if (!ec) {
        std::stringstream ss;
        ss << "Accepted connection from "
             << connection->socket().remote_endpoint().address().to_string() << ":"
             << connection->socket().remote_endpoint().port();
        m_logger->info(ss.str());

        connection->run();
    }
    else {
        std::stringstream ss;
        ss << "Failed to accept connection: "
            << ec.message();
        m_logger->error(ss.str());
    }

    startAccept();
}

} // namespace http
} // namespace manager
} // namespace ch