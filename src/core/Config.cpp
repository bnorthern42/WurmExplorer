#include "Config.hpp"
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <iostream>
#include <algorithm>

namespace treasure {
namespace core {

namespace {
    // Hardcoded clusters from python
    const std::vector<std::string> CLUSTER_ORDER = {"Southern", "Northern", "Epic"};
    
    const std::map<std::string, std::vector<std::string>> CLUSTER_SERVERS = {
        {"Southern", {"Chaos", "Independence", "Deliverance", "Exodus", "Celebration", "Pristine", "Release", "Xanadu"}},
        {"Northern", {"Harmony", "Melody", "Cadence", "Defiance"}},
        {"Epic", {"Affliction", "Desertion", "Elevation", "Serenity"}}
    };

    std::string getClusterForServer(const std::string& serverName) {
        for (const auto& [cluster, servers] : CLUSTER_SERVERS) {
            if (std::find(servers.begin(), servers.end(), serverName) != servers.end()) {
                return cluster;
            }
        }
        return "Southern"; // Default
    }
}

ConfigManager::ConfigManager() {
}

bool ConfigManager::loadConfig(const std::string& configPath) {
    try {
        YAML::Node config = YAML::LoadFile(configPath);
        if (!config["servers"]) return false;

        std::filesystem::path basePath(configPath);
        basePath = basePath.parent_path();

        YAML::Node serversNode = config["servers"];
        for (auto it = serversNode.begin(); it != serversNode.end(); ++it) {
            std::string name = it->first.as<std::string>();
            YAML::Node s = it->second;

            auto cfg = std::make_shared<ServerConfig>();
            cfg->name = name;
            
            if (s["map_image"]) {
                std::filesystem::path mapPath(s["map_image"].as<std::string>());
                if (mapPath.is_absolute()) {
                    cfg->map_image = mapPath.string();
                } else {
                    cfg->map_image = (basePath / mapPath).lexically_normal().string();
                }
            }

            if (s["map_size_tiles"]) cfg->map_size_tiles = s["map_size_tiles"].as<int>();
            
            if (s["scales"] && s["scales"].IsSequence()) {
                cfg->scales.clear();
                for (auto scale : s["scales"]) {
                    cfg->scales.push_back(scale.as<float>());
                }
            }
            if (cfg->scales.empty()) cfg->scales = {1.0f};

            if (s["topk"]) cfg->topk = s["topk"].as<int>();
            if (s["map_crop_auto"]) cfg->map_crop_auto = s["map_crop_auto"].as<bool>();
            
            servers[name] = cfg;
        }
        return true;
    } catch (const YAML::Exception& e) {
        std::cerr << "YAML parsing error: " << e.what() << "\n";
        return false;
    }
}

std::vector<std::string> ConfigManager::getAvailableClusters() const {
    std::vector<std::string> res;
    for (const auto& cluster : CLUSTER_ORDER) {
        auto serversForCluster = getServersForCluster(cluster);
        if (!serversForCluster.empty()) {
            res.push_back(cluster);
        }
    }
    return res.empty() ? CLUSTER_ORDER : res;
}

std::vector<std::string> ConfigManager::getServersForCluster(const std::string& cluster) const {
    std::vector<std::string> res;
    auto it = CLUSTER_SERVERS.find(cluster);
    if (it != CLUSTER_SERVERS.end()) {
        for (const auto& preferred : it->second) {
            if (servers.find(preferred) != servers.end()) {
                res.push_back(preferred);
            }
        }
    }
    
    // add extras
    std::vector<std::string> extras;
    for (const auto& [name, cfg] : servers) {
        if (getClusterForServer(name) == cluster) {
            if (std::find(res.begin(), res.end(), name) == res.end()) {
                extras.push_back(name);
            }
        }
    }
    std::sort(extras.begin(), extras.end());
    res.insert(res.end(), extras.begin(), extras.end());
    return res;
}

std::shared_ptr<ServerConfig> ConfigManager::getServerConfig(const std::string& name) const {
    auto it = servers.find(name);
    if (it != servers.end()) return it->second;
    return nullptr;
}

} // namespace core
} // namespace treasure
