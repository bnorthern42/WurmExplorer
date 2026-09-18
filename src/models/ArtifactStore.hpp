#pragma once

#include <string>
#include <vector>
#include <map>
#include "AnnotationStore.hpp" // For Point

namespace treasure {
namespace models {

struct ArtifactClue {
    std::string artifact;
    Point caster_tile;
    std::string facing;
    std::string band_label;
    float cone_width_deg = 120.0f;
};

class ArtifactStore {
public:
    ArtifactStore(const std::string& filepath);
    
    void load();
    void save();
    
    bool getCaster(const std::string& server, Point& outCaster) const;
    void setCaster(const std::string& server, const Point& caster);
    
    std::vector<ArtifactClue> getClues(const std::string& server, const std::string& artifact = "") const;
    void addClue(const std::string& server, const ArtifactClue& clue);
    void removeClue(const std::string& server, const std::string& artifact, int index);
    
    void clearArtifact(const std::string& server, const std::string& artifact);
    void clearServer(const std::string& server);

private:
    std::string filepath;
    std::map<std::string, Point> casters;
    std::map<std::string, std::map<std::string, std::vector<ArtifactClue>>> clues;
};

} // namespace models
} // namespace treasure
