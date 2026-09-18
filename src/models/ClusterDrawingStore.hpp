#pragma once

#include <string>
#include <vector>
#include "AnnotationStore.hpp" // For Point

namespace treasure {
namespace models {

struct ClusterDrawingItem {
    std::string tool; // polyline | rectangle | circle | arrow | text
    std::string color;
    int width = 2;
    std::string label = "";
    std::vector<Point> points;
    bool visible = true;
    
    std::string font_family = "";
    int font_size = 14;
    bool font_bold = false;
    bool font_italic = false;
};

struct ClusterDrawingObject {
    std::string id;
    std::string cluster;
    std::string plan_name;
    std::string layer;
    std::string name;
    std::vector<ClusterDrawingItem> items;
    bool visible = true;
};

class ClusterDrawingStore {
public:
    ClusterDrawingStore(const std::string& filepath);
    
    void load();
    void save();
    
    std::vector<ClusterDrawingObject> getByCluster(const std::string& cluster) const;
    std::vector<std::string> getPlanNames(const std::string& cluster) const;
    std::vector<ClusterDrawingObject> getByPlan(const std::string& cluster, const std::string& planName) const;
    std::optional<ClusterDrawingObject> get(const std::string& id) const;
    
    void add(const ClusterDrawingObject& obj);
    void update(const ClusterDrawingObject& obj);
    void remove(const std::string& id);
    void removePlan(const std::string& cluster, const std::string& planName);

private:
    std::string filepath;
    std::vector<ClusterDrawingObject> objects;
};

} // namespace models
} // namespace treasure
