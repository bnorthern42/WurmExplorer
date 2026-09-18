#include "DrawingStore.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <algorithm>

using json = nlohmann::json;

namespace treasure {
namespace models {

void to_json(json& j, const DrawingItem& i) {
    j = json{
        {"tool", i.tool},
        {"color", i.color},
        {"width", i.width},
        {"label", i.label},
        {"points", i.points},
        {"visible", i.visible},
        {"font_family", i.font_family},
        {"font_size", i.font_size},
        {"font_bold", i.font_bold},
        {"font_italic", i.font_italic}
    };
}

void from_json(const json& j, DrawingItem& i) {
    if (j.contains("tool")) j.at("tool").get_to(i.tool);
    if (j.contains("color")) j.at("color").get_to(i.color);
    if (j.contains("width")) j.at("width").get_to(i.width);
    if (j.contains("label")) j.at("label").get_to(i.label);
    if (j.contains("points")) j.at("points").get_to(i.points);
    if (j.contains("visible")) j.at("visible").get_to(i.visible);
    if (j.contains("font_family")) j.at("font_family").get_to(i.font_family);
    if (j.contains("font_size")) j.at("font_size").get_to(i.font_size);
    if (j.contains("font_bold")) j.at("font_bold").get_to(i.font_bold);
    if (j.contains("font_italic")) j.at("font_italic").get_to(i.font_italic);
}

void to_json(json& j, const DrawingObject& o) {
    j = json{
        {"id", o.id},
        {"server", o.server},
        {"plan_name", o.plan_name},
        {"layer", o.layer},
        {"name", o.name},
        {"items", o.items},
        {"visible", o.visible}
    };
}

void from_json(const json& j, DrawingObject& o) {
    if (j.contains("id")) j.at("id").get_to(o.id);
    if (j.contains("server")) j.at("server").get_to(o.server);
    if (j.contains("plan_name")) j.at("plan_name").get_to(o.plan_name);
    if (j.contains("layer")) j.at("layer").get_to(o.layer);
    if (j.contains("name")) j.at("name").get_to(o.name);
    if (j.contains("visible")) j.at("visible").get_to(o.visible);
    
    if (j.contains("items")) {
        j.at("items").get_to(o.items);
    } else {
        // Fallback for single object legacy data
        DrawingItem item;
        if (j.contains("tool")) j.at("tool").get_to(item.tool);
        if (j.contains("color")) j.at("color").get_to(item.color);
        if (j.contains("width")) j.at("width").get_to(item.width);
        if (j.contains("label")) j.at("label").get_to(item.label);
        if (j.contains("points")) j.at("points").get_to(item.points);
        o.items.push_back(item);
    }
}

DrawingStore::DrawingStore(const std::string& filepath) : filepath(filepath) {
    load();
}

void DrawingStore::load() {
    std::ifstream file(filepath);
    if (!file.is_open()) return;
    try {
        json j;
        file >> j;
        objects = j.get<std::vector<DrawingObject>>();
    } catch (const json::exception& e) {
        std::cerr << "DrawingStore JSON parse error: " << e.what() << '\n';
        objects.clear();
    }
}

void DrawingStore::save() {
    std::ofstream file(filepath);
    if (file.is_open()) {
        json j = objects;
        file << j.dump(2);
    }
}

std::vector<DrawingObject> DrawingStore::getByServer(const std::string& server) const {
    std::vector<DrawingObject> res;
    for (const auto& obj : objects) {
        if (obj.server == server) res.push_back(obj);
    }
    return res;
}

std::vector<std::string> DrawingStore::getPlanNames(const std::string& server) const {
    std::vector<std::string> names;
    for (const auto& obj : objects) {
        if (obj.server == server && !obj.plan_name.empty()) {
            if (std::find(names.begin(), names.end(), obj.plan_name) == names.end()) {
                names.push_back(obj.plan_name);
            }
        }
    }
    std::sort(names.begin(), names.end());
    return names;
}

std::vector<DrawingObject> DrawingStore::getByPlan(const std::string& server, const std::string& plan_name) const {
    std::vector<DrawingObject> res;
    for (const auto& obj : objects) {
        if (obj.server == server && obj.plan_name == plan_name) {
            res.push_back(obj);
        }
    }
    return res;
}

void DrawingStore::add(const DrawingObject& obj) {
    objects.push_back(obj);
    save();
}

void DrawingStore::update(const DrawingObject& updated) {
    for (auto& obj : objects) {
        if (obj.id == updated.id) {
            obj = updated;
            save();
            return;
        }
    }
}

void DrawingStore::remove(const std::string& id) {
    auto it = objects.begin();
    while (it != objects.end()) {
        if (it->id == id) {
            it = objects.erase(it);
        } else {
            ++it;
        }
    }
    save();
}

void DrawingStore::deletePlan(const std::string& server, const std::string& plan_name) {
    auto it = objects.begin();
    while (it != objects.end()) {
        if (it->server == server && it->plan_name == plan_name) {
            it = objects.erase(it);
        } else {
            ++it;
        }
    }
    save();
}

} // namespace models
} // namespace treasure
