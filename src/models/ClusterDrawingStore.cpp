#include "ClusterDrawingStore.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <algorithm>

using json = nlohmann::json;

namespace treasure {
namespace models {

void to_json(json& j, const ClusterDrawingItem& i) {
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

void from_json(const json& j, ClusterDrawingItem& i) {
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

void to_json(json& j, const ClusterDrawingObject& o) {
    j = json{
        {"id", o.id},
        {"cluster", o.cluster},
        {"plan_name", o.plan_name},
        {"layer", o.layer},
        {"name", o.name},
        {"items", o.items},
        {"visible", o.visible}
    };
}

void from_json(const json& j, ClusterDrawingObject& o) {
    if (j.contains("id")) j.at("id").get_to(o.id);
    if (j.contains("cluster")) j.at("cluster").get_to(o.cluster);
    if (j.contains("plan_name")) j.at("plan_name").get_to(o.plan_name);
    if (j.contains("layer")) j.at("layer").get_to(o.layer);
    if (j.contains("name")) j.at("name").get_to(o.name);
    if (j.contains("visible")) j.at("visible").get_to(o.visible);
    
    if (j.contains("items")) {
        j.at("items").get_to(o.items);
    } else {
        // Fallback for single object legacy data
        ClusterDrawingItem item;
        if (j.contains("tool")) j.at("tool").get_to(item.tool);
        if (j.contains("color")) j.at("color").get_to(item.color);
        if (j.contains("width")) j.at("width").get_to(item.width);
        if (j.contains("label")) j.at("label").get_to(item.label);
        if (j.contains("points")) j.at("points").get_to(item.points);
        o.items.push_back(item);
    }
}

ClusterDrawingStore::ClusterDrawingStore(const std::string& filepath) : filepath(filepath) {
    load();
}

void ClusterDrawingStore::load() {
    std::ifstream file(filepath);
    if (!file.is_open()) return;
    try {
        json j;
        file >> j;
        objects = j.get<std::vector<ClusterDrawingObject>>();
    } catch (const json::exception& e) {
        std::cerr << "ClusterDrawingStore JSON parse error: " << e.what() << '\n';
        objects.clear();
    }
}

void ClusterDrawingStore::save() {
    std::ofstream file(filepath);
    if (file.is_open()) {
        json j = objects;
        file << j.dump(2);
    }
}

std::optional<ClusterDrawingObject> ClusterDrawingStore::get(const std::string& id) const {
    for (const auto& obj : objects) {
        if (obj.id == id) return obj;
    }
    return std::nullopt;
}

std::vector<ClusterDrawingObject> ClusterDrawingStore::getByCluster(const std::string& cluster) const {
    std::vector<ClusterDrawingObject> res;
    for (const auto& obj : objects) {
        if (obj.cluster == cluster) res.push_back(obj);
    }
    return res;
}

std::vector<std::string> ClusterDrawingStore::getPlanNames(const std::string& cluster) const {
    std::vector<std::string> names;
    for (const auto& obj : objects) {
        if (obj.cluster == cluster && !obj.plan_name.empty()) {
            if (std::find(names.begin(), names.end(), obj.plan_name) == names.end()) {
                names.push_back(obj.plan_name);
            }
        }
    }
    std::sort(names.begin(), names.end());
    return names;
}

std::vector<ClusterDrawingObject> ClusterDrawingStore::getByPlan(const std::string& cluster, const std::string& plan_name) const {
    std::vector<ClusterDrawingObject> res;
    for (const auto& obj : objects) {
        if (obj.cluster == cluster && obj.plan_name == plan_name) {
            res.push_back(obj);
        }
    }
    return res;
}

void ClusterDrawingStore::add(const ClusterDrawingObject& obj) {
    objects.push_back(obj);
    save();
}

void ClusterDrawingStore::update(const ClusterDrawingObject& updated) {
    for (auto& obj : objects) {
        if (obj.id == updated.id) {
            obj = updated;
            save();
            return;
        }
    }
}

void ClusterDrawingStore::remove(const std::string& id) {
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

void ClusterDrawingStore::removePlan(const std::string& cluster, const std::string& plan_name) {
    auto it = objects.begin();
    while (it != objects.end()) {
        if (it->cluster == cluster && it->plan_name == plan_name) {
            it = objects.erase(it);
        } else {
            ++it;
        }
    }
    save();
}

} // namespace models
} // namespace treasure
