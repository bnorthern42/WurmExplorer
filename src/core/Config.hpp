#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace treasure {
namespace core {

struct ServerConfig {
    std::string name;
    std::string map_image; // Base map image path
    int map_size_tiles = 1024;
    std::vector<float> scales = {1.0f};

    int mask_compass_px = 120;
    int mask_border_px = 8;
    std::vector<float> x_template_fracs;

    int canny1 = 45;
    int canny2 = 120;
    int blur_ksize = 5;

    float score_weight_edges = 0.7f;
    float score_weight_gray = 0.3f;

    bool avoid_water = true;
    bool map_crop_auto = false;
    int topk = 7;

    bool hint_use_roi = true;
    bool hint_soft_bias = true;
    float hint_bias_sigma_tiles = 250.0f;
    float hint_bias_min_weight = 0.10f;

    std::string server_mode = "pve";
    bool supports_kingdoms = false;
    int guard_tower_influence_radius_tiles = 50;
};

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager() = default;

    bool loadConfig(const std::string& configPath);
    
    std::vector<std::string> getAvailableClusters() const;
    std::vector<std::string> getServersForCluster(const std::string& cluster) const;
    
    std::shared_ptr<ServerConfig> getServerConfig(const std::string& name) const;

private:
    std::map<std::string, std::shared_ptr<ServerConfig>> servers;
};

} // namespace core
} // namespace treasure
