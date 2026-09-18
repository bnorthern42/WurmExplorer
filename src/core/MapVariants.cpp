#include "MapVariants.hpp"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace treasure {
namespace core {

const std::vector<std::string> KNOWN_MAP_TYPES = {"classic", "topo", "terrain"};

std::vector<std::string> ServerMapVariants::availableTypes() const {
    std::vector<std::string> types;
    for (const auto& t : KNOWN_MAP_TYPES) {
        if (paths.find(t) != paths.end()) {
            types.push_back(t);
        }
    }
    return types;
}

bool ServerMapVariants::hasType(const std::string& type) const {
    return paths.find(type) != paths.end();
}

std::string ServerMapVariants::pathFor(const std::string& type) const {
    auto it = paths.find(type);
    if (it != paths.end()) return it->second;
    it = paths.find(default_type);
    if (it != paths.end()) return it->second;
    if (!paths.empty()) return paths.begin()->second;
    return "";
}

static std::string toLower(const std::string& str) {
    std::string out = str;
    std::transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

static std::string inferTypeFromName(const std::string& serverName, const std::string& filename) {
    std::string lowerName = toLower(filename);
    std::string serverLower = toLower(serverName);

    for (const auto& t : KNOWN_MAP_TYPES) {
        std::string token = serverLower + "-" + t + "-";
        if (lowerName.find(token) != std::string::npos) {
            return t;
        }
    }
    return "";
}

static std::string extractDateKey(const std::string& serverName, const std::string& mapType, const std::string& stem) {
    std::string lowerStem = toLower(stem);
    std::string token = toLower(serverName) + "-" + mapType + "-";
    auto idx = lowerStem.find(token);
    if (idx == std::string::npos) return "";
    return stem.substr(idx + token.length());
}

std::shared_ptr<ServerMapVariants> discoverServerMapVariants(const std::shared_ptr<ServerConfig>& cfg) {
    if (!cfg) return nullptr;

    fs::path basePath(cfg->map_image);
    fs::path directory = basePath.parent_path();
    
    auto variants = std::make_shared<ServerMapVariants>();
    variants->server = cfg->name;

    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        variants->paths["classic"] = basePath.string();
        variants->default_type = "classic";
        return variants;
    }

    std::vector<fs::path> candidates;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            candidates.push_back(entry.path());
        }
    }
    std::sort(candidates.begin(), candidates.end());

    for (const auto& cand : candidates) {
        std::string mapType = inferTypeFromName(cfg->name, cand.filename().string());
        if (mapType.empty()) continue;

        auto it = variants->paths.find(mapType);
        if (it == variants->paths.end()) {
            variants->paths[mapType] = cand.string();
        } else {
            std::string existingDate = extractDateKey(cfg->name, mapType, fs::path(it->second).stem().string());
            std::string newDate = extractDateKey(cfg->name, mapType, cand.stem().string());
            if (newDate >= existingDate) {
                variants->paths[mapType] = cand.string();
            }
        }
    }

    std::string inferredDefault = inferTypeFromName(cfg->name, basePath.filename().string());
    if (!inferredDefault.empty() && variants->paths.find(inferredDefault) == variants->paths.end()) {
        variants->paths[inferredDefault] = basePath.string();
    }

    if (variants->paths.empty()) {
        variants->paths["classic"] = basePath.string();
    }

    if (!inferredDefault.empty() && variants->paths.find(inferredDefault) != variants->paths.end()) {
        variants->default_type = inferredDefault;
    } else if (variants->paths.find("classic") != variants->paths.end()) {
        variants->default_type = "classic";
    } else {
        variants->default_type = variants->paths.begin()->first;
    }

    return variants;
}

} // namespace core
} // namespace treasure
