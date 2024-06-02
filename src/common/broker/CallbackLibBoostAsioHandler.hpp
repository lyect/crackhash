#pragma once

#include <amqpcpp.h>
#include <amqpcpp/linux_tcp.h>
#include <amqpcpp/libboostasio.h>

#include <boost/function.hpp>

namespace ch {
namespace common {

using OnReadyCallback = boost::function<void(void)>;

class CallbackLibBoostAsioHandler : public AMQP::LibBoostAsioHandler {

public:

    CallbackLibBoostAsioHandler(
            boost::asio::io_context &ioc,
            OnReadyCallback onReadyCallback);
    
    CallbackLibBoostAsioHandler(LibBoostAsioHandler &&that) = delete;
    CallbackLibBoostAsioHandler(const LibBoostAsioHandler &that) = delete;

    virtual void onReady(AMQP::TcpConnection *connection) override;
    
private:

    OnReadyCallback m_onReadyCallback;
};

} // namespace common
} // namespace ch