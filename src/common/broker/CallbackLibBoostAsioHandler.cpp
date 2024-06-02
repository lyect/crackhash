#include "CallbackLibBoostAsioHandler.hpp"

namespace ch {
namespace common {

CallbackLibBoostAsioHandler::CallbackLibBoostAsioHandler(
        boost::asio::io_context &ioc,
        OnReadyCallback onReadyCallback)
    : AMQP::LibBoostAsioHandler(ioc)
    , m_onReadyCallback{onReadyCallback}
{}

void CallbackLibBoostAsioHandler::onReady(
    __attribute__((unused)) AMQP::TcpConnection *connection
) {
    if (m_onReadyCallback) {
        m_onReadyCallback();
    }
}

} // namespace common
} // namespace ch