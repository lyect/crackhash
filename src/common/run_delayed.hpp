#pragma once

#include <boost/function.hpp>
#include <boost/asio/io_context.hpp>

namespace ch {
namespace common {

using DelayedRunnerRoutine = boost::function<void()>;

void runDelayed(
    boost::asio::io_context &ioc,
    std::uint16_t            delay,
    DelayedRunnerRoutine     routine);

} // namespace common
} // namespace ch