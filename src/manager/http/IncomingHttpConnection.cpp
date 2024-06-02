#include "IncomingHttpConnection.hpp"

#include "manager/config/ManagerConfig.hpp"

#include <boost/core/ignore_unused.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/asio.hpp>

#include <sstream>

namespace ch {
namespace manager {
namespace http {

IncomingHttpConnection::IncomingHttpConnection(
    boost::asio::io_context   &ioc,
    CreateResponseFunctorType  createResponseFunctor)
        : m_socket{ioc}
        , m_buffer{config::ManagerConfig::getInstance()->incomingHttpConnectionConfig().readBufferSize()}
        , m_deadlineTimer{ioc}
        , m_logger{spdlog::get(config::ManagerConfig::getInstance()->loggerConfig().loggerName())}
        , m_createResponseFunctor{std::move(createResponseFunctor)}
        , m_closed{false}
{}

IncomingHttpConnection::~IncomingHttpConnection() {
    if (m_closed) {
        m_ss.str("");
        m_ss << "Closed connection with "
             << m_remoteIpAddress << ":"
             << m_remoteIpPort;
        m_logger->info(m_ss.str());
        m_logger->flush();
    }
}

void IncomingHttpConnection::run() {

    m_remoteIpAddress = m_socket.remote_endpoint().address().to_string();
    m_remoteIpPort = m_socket.remote_endpoint().port();

    startRequestRead();
    startDeadlineWait(); 
}

void IncomingHttpConnection::restartDeadlineTimer() {
    m_deadlineTimer.expires_from_now(
        boost::posix_time::seconds(
            config::ManagerConfig::getInstance()->incomingHttpConnectionConfig().timeout()));
}

void IncomingHttpConnection::startRequestRead() {
    // If connection has been closed, there is no need to start reading.
    //  Each further attempt to read will fail
    if (m_closed) {
        return;
    }

    restartDeadlineTimer();

    boost::beast::http::async_read(
        m_socket,
        m_buffer,
        m_request,
        boost::beast::bind_front_handler(
            &IncomingHttpConnection::handleRequestRead, shared_from_this()));
}

void IncomingHttpConnection::handleRequestRead(
    boost::beast::error_code ec,
    std::size_t bytes_transferred
) {
    boost::ignore_unused(bytes_transferred);
    
    if (!ec) {

        if (bytes_transferred == 0) {
            m_ss.str("");
            m_ss << "Got empty request from "
                 << m_remoteIpAddress << ":"
                 << m_remoteIpPort;
            m_logger->info(m_ss.str());
            close("");
            return;
        }

        m_ss.str("");
        m_ss << "Got \""
             << boost::beast::http::to_string(m_request.method())
             << "\" request from "
             << m_remoteIpAddress << ":"
             << m_remoteIpPort;
        m_logger->info(m_ss.str());

        if (m_createResponseFunctor == nullptr) {
            m_ss.str("");
            m_ss << "CreateResponseFunctor is NULL for connection with "
                 << m_remoteIpAddress << ":"
                 << m_remoteIpPort;
            close(m_ss.str());
            return;
        }
        
        m_response = m_createResponseFunctor(m_request);
        startResponseWrite();
        
        return;
    }

    // If read has failed and connection has been already closed,
    //  there is no need to make message and close again
    if (m_closed) {
        return;
    }

    // Close connection if read has failed and
    //  connection hasn't been closed yet
    m_ss.str("");
    m_ss << "Got return code "
            << ec.value() << "(" << ec.message() << ")"
            << " while async_read from "
            << m_remoteIpAddress << ":"
            << m_remoteIpPort;
    close(m_ss.str());
}

void IncomingHttpConnection::startResponseWrite() {
    // If connection has been closed, there is no need to start writing.
    //  Each further attempt to write will fail
    if (m_closed) {
        return;
    }

    m_response.content_length(m_response.body().size());

    boost::beast::http::async_write(
        m_socket,
        m_response,
        boost::beast::bind_front_handler(
            &IncomingHttpConnection::handleResponseWrite, shared_from_this()));
}

void IncomingHttpConnection::handleResponseWrite(
    boost::beast::error_code ec,
    std::size_t bytes_transferred
) {
    boost::ignore_unused(bytes_transferred);

    if (!ec) {
        
        if (bytes_transferred == 0) {
            m_ss.str("");
            m_ss << "Sent nothing to "
                 << m_remoteIpAddress << ":"
                 << m_remoteIpPort;
            m_logger->info(m_ss.str());
            close("");
            return;
        }

        startRequestRead();
        return;
    }
    
    // If write has failed and connection has been already closed,
    //  there is no need to make message and close again
    if (m_closed) {
        return;
    }

    // Close connection if write has failed and
    //  connection hasn't been closed yet
    m_ss.str("");
    m_ss << "Got return code "
            << ec.value() << "(" << ec.message() << ")"
            << " while async_write to "
            << m_remoteIpAddress << ":"
            << m_remoteIpPort;
    close(m_ss.str());
}

void IncomingHttpConnection::startDeadlineWait() {
    // If connection has been closed, there is no need to start timer.
    //  It will just increase lifetime of already closed connection.
    if (m_closed) {
        return;
    }

    m_deadlineTimer.async_wait(
        boost::beast::bind_front_handler(
            &IncomingHttpConnection::handleDeadlineExpired, shared_from_this()));
}

void IncomingHttpConnection::handleDeadlineExpired(boost::beast::error_code ec) {
    // If timer expired after the connection closed,
    //  there is no need to check whether it failed or not.
    // We just don't care about it anymore.
    if (m_closed) {
        return;
    }

    if (!ec) {
        m_ss.str("");
        m_ss << "Timeout on connection with "
           << m_remoteIpAddress << ":"
           << m_remoteIpPort;
        close(m_ss.str());
        return;
    }

    // ec.failed() will return true whether timer was cancelled or not.
    // So, we need to use another approach here

    if (ec.value() == boost::asio::error::operation_aborted && m_closed) {
        // Timer was aborted because it was cancelled (since connection has been closed).
        // Therefore, there is no need to try to close connection again, just return.
        return;
    }

    if (ec.value() == boost::asio::error::operation_aborted && !m_closed) {
        // Timer was aborted because it was restarted (since connection hasn't been closed).
        // Need to restart timer.
        startDeadlineWait();
        return;
    }

    m_ss.str("");
    m_ss << "Got return code "
            << ec.value() << "(" << ec.message() << ")"
            << " while async_wait on timer for connection with "
            << m_remoteIpAddress << ":"
            << m_remoteIpPort;
    close(m_ss.str());
    return;
}

void IncomingHttpConnection::close(const std::string &message) {
    if (m_closed) {
        m_ss.str("");
        m_ss << "Connection with "
             << m_remoteIpAddress
             << ":"
             << m_remoteIpPort
             << " already closed.";
        if (!message.empty()) {
            m_ss << " \""
                 << message
                 << "\" expected to be printed";
            m_logger->info(m_ss.str());
        }
        m_deadlineTimer.cancel();
        return;
    }
    m_closed = true;
    m_deadlineTimer.cancel();
    m_socket.close();
    if (!message.empty()) {
        m_logger->info(message);
    }
}

} // namespace http
} // namespace manager
} // namespace ch
