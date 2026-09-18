#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include "Config.hpp"

namespace treasure {
namespace core {

extern const std::vector<std::string> KNOWN_MAP_TYPES;

struct ServerMapVariants {
    std::string server;
    std::map<std::string, std::string> paths; // type -> path
    std::string default_type;

    std::vector<std::string> availableTypes() const;
    bool hasType(const std::string& type) const;
    std::string pathFor(const std::string& type) const;
};

std::shared_ptr<ServerMapVariants> discoverServerMapVariants(const std::shared_ptr<ServerConfig>& cfg);

} // namespace core
} // namespace treasure
