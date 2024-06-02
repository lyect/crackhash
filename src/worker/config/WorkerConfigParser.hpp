#pragma once

#include "tp/nlohmann/json.hpp"

#include <string>

namespace ch {
namespace worker {
namespace config {

class WorkerConfigParser {

public:

    static bool read(const std::string &configFilePath);

private:

    static void parseLoggerConfig(const nlohmann::json &jsonManagerConfig);
    static void parseMessageBrokerConfig(const nlohmann::json &jsonManagerConfig);
    static void parseWorkerParameters(const nlohmann::json &jsonManagerConfig);
};

} // namespace config
} // namespace worker
} // namespace ch