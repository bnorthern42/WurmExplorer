#pragma once

#include <string>
#include <vector>
#include "AnnotationStore.hpp" // For Point

namespace treasure {
namespace models {

struct DrawingItem {
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

struct DrawingObject {
    std::string id;
    std::string server;
    std::string plan_name;
    std::string layer;
    std::string name;
    std::vector<DrawingItem> items;
    bool visible = true;
};

class DrawingStore {
public:
    DrawingStore(const std::string& filepath);
    
    void load();
    void save();
    
    std::vector<DrawingObject> getByServer(const std::string& server) const;
    std::vector<std::string> getPlanNames(const std::string& server) const;
    std::vector<DrawingObject> getByPlan(const std::string& server, const std::string& plan_name) const;
    
    void add(const DrawingObject& obj);
    void update(const DrawingObject& obj);
    void remove(const std::string& id);
    void deletePlan(const std::string& server, const std::string& plan_name);

private:
    std::string filepath;
    std::vector<DrawingObject> objects;
};

} // namespace models
} // namespace treasure
