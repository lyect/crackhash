#include "run_delayed.hpp"

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/post.hpp>
#include <boost/bind/bind.hpp>

namespace ch {
namespace common {

class DelayedRunner : public std::enable_shared_from_this<DelayedRunner> {

public:

    DelayedRunner(
            boost::asio::io_context &ioc,
            std::uint16_t            delay,
            DelayedRunnerRoutine     routine)
        : m_ioc{ioc}
        , m_timer{m_ioc}
        , m_delay{delay}
        , m_routine{std::move(routine)}
    {}

    void run() {
        m_timer.expires_from_now(m_delay);
        boost::asio::post(
            m_ioc,
            boost::bind(
                &DelayedRunner::startWait, shared_from_this()));
    }

private:

    void startWait() {
        
        m_timer.async_wait(
            boost::bind(
                &DelayedRunner::onTimeout, shared_from_this(),
                boost::placeholders::_1));
    }

    void onTimeout(
        __attribute__((unused)) boost::system::error_code ec
    ) {
        // TODO:
        // Restart if cancelled
        m_routine();
    }

    boost::asio::io_context     &m_ioc;
    boost::asio::deadline_timer  m_timer;
    boost::posix_time::seconds   m_delay;
    DelayedRunnerRoutine         m_routine;
};

void runDelayed(
    boost::asio::io_context &ioc,
    std::uint16_t            delay,
    DelayedRunnerRoutine     routine
) {
    std::make_shared<DelayedRunner>(ioc, delay, std::move(routine))->run();
}

} // namespace common
} // namespace ch