#pragma once

#include "tp/nlohmann/json.hpp"

#include <string>

namespace ch {
namespace manager {
namespace config {

class ManagerConfigParser {

public:

    static bool read(const std::string &configFilePath);

private:

    static void parseLoggerConfig(const nlohmann::json &jsonManagerConfig);
    static void parseIncomingHttpConnectionConfig(const nlohmann::json &jsonManagerConfig);
    static void parseMessageBrokerConfig(const nlohmann::json &jsonManagerConfig);
    static void parseCrackHashRequestDatabaseConfig(const nlohmann::json &jsonManagerConfig);
    static void parseManagerParameters(const nlohmann::json &jsonManagerConfig);
};

} // namespace config
} // namespace manager
} // namespace ch