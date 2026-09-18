#include "ArtifactStore.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace treasure {
namespace models {

void to_json(json& j, const ArtifactClue& c) {
    j = json{
        {"artifact", c.artifact},
        {"caster_tile", {c.caster_tile.x, c.caster_tile.y}},
        {"facing", c.facing},
        {"band_label", c.band_label},
        {"cone_width_deg", c.cone_width_deg}
    };
}

void from_json(const json& j, ArtifactClue& c) {
    j.at("artifact").get_to(c.artifact);
    
    if (j.contains("caster_tile") && j.at("caster_tile").is_array() && j.at("caster_tile").size() >= 2) {
        j.at("caster_tile")[0].get_to(c.caster_tile.x);
        j.at("caster_tile")[1].get_to(c.caster_tile.y);
    } else {
        c.caster_tile.x = 0; c.caster_tile.y = 0;
    }
    
    j.at("facing").get_to(c.facing);
    j.at("band_label").get_to(c.band_label);
    if (j.contains("cone_width_deg")) {
        j.at("cone_width_deg").get_to(c.cone_width_deg);
    }
}

ArtifactStore::ArtifactStore(const std::string& filepath) : filepath(filepath) {
    load();
}

void ArtifactStore::load() {
    std::ifstream file(filepath);
    if (!file.is_open()) return;
    try {
        json data;
        file >> data;
        
        if (data.contains("casters")) {
            for (const auto& [server, coords] : data["casters"].items()) {
                if (coords.is_array() && coords.size() >= 2) {
                    casters[server] = Point{coords[0].get<float>(), coords[1].get<float>()};
                }
            }
        }
        
        if (data.contains("clues")) {
            for (const auto& [server, artMap] : data["clues"].items()) {
                if (artMap.is_object()) {
                    for (const auto& [artifact, clueList] : artMap.items()) {
                        if (clueList.is_array()) {
                            clues[server][artifact] = clueList.get<std::vector<ArtifactClue>>();
                        }
                    }
                }
            }
        }
    } catch (const json::exception& e) {
        std::cerr << "ArtifactStore JSON parse error: " << e.what() << '\n';
        casters.clear();
        clues.clear();
    }
}

void ArtifactStore::save() {
    std::ofstream file(filepath);
    if (file.is_open()) {
        json j;
        j["casters"] = json::object();
        for (const auto& [server, pt] : casters) {
            j["casters"][server] = {pt.x, pt.y};
        }
        j["clues"] = clues;
        file << j.dump(2);
    }
}

bool ArtifactStore::getCaster(const std::string& server, Point& outCaster) const {
    auto it = casters.find(server);
    if (it != casters.end()) {
        outCaster = it->second;
        return true;
    }
    return false;
}

void ArtifactStore::setCaster(const std::string& server, const Point& caster) {
    casters[server] = caster;
    save();
}

std::vector<ArtifactClue> ArtifactStore::getClues(const std::string& server, const std::string& artifact) const {
    std::vector<ArtifactClue> result;
    auto sIt = clues.find(server);
    if (sIt != clues.end()) {
        if (artifact.empty()) {
            for (const auto& [art, cl] : sIt->second) {
                result.insert(result.end(), cl.begin(), cl.end());
            }
        } else {
            auto aIt = sIt->second.find(artifact);
            if (aIt != sIt->second.end()) {
                result = aIt->second;
            }
        }
    }
    return result;
}

void ArtifactStore::addClue(const std::string& server, const ArtifactClue& clue) {
    clues[server][clue.artifact].push_back(clue);
    save();
}

void ArtifactStore::removeClue(const std::string& server, const std::string& artifact, int index) {
    auto& list = clues[server][artifact];
    if (index >= 0 && static_cast<size_t>(index) < list.size()) {
        list.erase(list.begin() + index);
        save();
    }
}

void ArtifactStore::clearArtifact(const std::string& server, const std::string& artifact) {
    clues[server][artifact].clear();
    save();
}

void ArtifactStore::clearServer(const std::string& server) {
    clues[server].clear();
    save();
}

} // namespace models
} // namespace treasure
