#pragma once

#include "common/config/CommonConfig.hpp"

namespace ch {
namespace worker {
namespace config {

class WorkerConfig : public common::config::CommonConfig {

    friend class WorkerConfigParser;

public:

    static WorkerConfig *getInstance() {
        return static_cast<WorkerConfig *>(common::config::CommonConfig::getInstance());
    }

    std::uint16_t startDelay() const {
        return m_startDelay;
    }

    std::size_t nThreads() const {
        return m_nThreads;
    }

    std::size_t burst() const {
        return m_burst;
    }

private:

    std::uint16_t                  m_startDelay;
    std::size_t                    m_nThreads;
    std::size_t                    m_burst;
};

} // namespace config
} // namespace worker
} // namespace ch