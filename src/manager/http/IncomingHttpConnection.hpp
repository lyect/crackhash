#pragma once

#include "manager/http/types.hpp"

#include "tp/spdlog/spdlog.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/beast/core/flat_buffer.hpp>

#include <memory>
#include <chrono>

namespace ch {
namespace manager {
namespace http {

using RequestType = boost::beast::http::request<boost::beast::http::string_body>;
using ResponseType = boost::beast::http::response<boost::beast::http::string_body>;
using CreateResponseFunctorType = boost::function<ResponseType(const RequestType &)>;

class IncomingHttpConnection : public std::enable_shared_from_this<IncomingHttpConnection> {

public:

    IncomingHttpConnection(
        boost::asio::io_context   &ioc,
        CreateResponseFunctorType  createResponseFunctor);
    
    ~IncomingHttpConnection();

    boost::asio::ip::tcp::socket &socket() {
        return m_socket;
    }

    void run();

private:

    boost::asio::ip::tcp::socket m_socket;
    boost::beast::flat_buffer m_buffer;
    boost::asio::deadline_timer m_deadlineTimer;

    std::stringstream m_ss;

    std::shared_ptr<spdlog::logger> m_logger;

    CreateResponseFunctorType m_createResponseFunctor;
    RequestType m_request;
    ResponseType m_response;

    std::string m_remoteIpAddress;
    boost::asio::ip::port_type m_remoteIpPort;

    bool m_closed;
    
    void restartDeadlineTimer();

    void startRequestRead();
    void handleRequestRead(
        boost::beast::error_code ec,
        std::size_t bytes_transferred);

    void startResponseWrite();
    void handleResponseWrite(
        boost::beast::error_code ec,
        std::size_t bytes_transferred);

    void startDeadlineWait();
    void handleDeadlineExpired(boost::beast::error_code ec);

    void close(const std::string &message);
};

} // namespace http
} // namespace manager
} // namespace ch