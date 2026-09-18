#include "AnnotationStore.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace treasure {
namespace models {

void to_json(json& j, const Point& p) {
    j = json::array({p.x, p.y});
}

void from_json(const json& j, Point& p) {
    if (j.is_array() && j.size() >= 2) {
        j.at(0).get_to(p.x);
        j.at(1).get_to(p.y);
    } else {
        p.x = 0; p.y = 0;
    }
}

void to_json(json& j, const Annotation& a) {
    j = json{
        {"id", a.id},
        {"server", a.server},
        {"type", a.type},
        {"status", a.status},
        {"name", a.name},
        {"notes", a.notes},
        {"tags", a.tags},
        {"points", a.points},
        {"source", a.source},
        {"source_name", a.source_name},
        {"source_key", a.source_key},
        {"source_sheet", a.source_sheet},
        {"category", a.category},
        {"imported_kind", a.imported_kind},
        {"visible", a.visible},
        {"kingdom", a.kingdom},
        {"influence_radius", a.influence_radius}
    };
}

void from_json(const json& j, Annotation& a) {
    j.at("id").get_to(a.id);
    j.at("server").get_to(a.server);
    j.at("type").get_to(a.type);
    j.at("status").get_to(a.status);
    j.at("name").get_to(a.name);
    
    if (j.contains("notes")) j.at("notes").get_to(a.notes);
    if (j.contains("tags")) j.at("tags").get_to(a.tags);
    if (j.contains("points")) j.at("points").get_to(a.points);
    if (j.contains("source")) j.at("source").get_to(a.source);
    if (j.contains("source_name")) j.at("source_name").get_to(a.source_name);
    if (j.contains("source_key")) j.at("source_key").get_to(a.source_key);
    if (j.contains("source_sheet")) j.at("source_sheet").get_to(a.source_sheet);
    if (j.contains("category")) j.at("category").get_to(a.category);
    if (j.contains("imported_kind")) j.at("imported_kind").get_to(a.imported_kind);
    if (j.contains("visible")) j.at("visible").get_to(a.visible);
    if (j.contains("kingdom")) j.at("kingdom").get_to(a.kingdom);
    if (j.contains("influence_radius")) j.at("influence_radius").get_to(a.influence_radius);
}

AnnotationStore::AnnotationStore(const std::string& filepath) : filepath(filepath) {
    load();
}

void AnnotationStore::load() {
    std::ifstream file(filepath);
    if (!file.is_open()) return;
    try {
        json j;
        file >> j;
        annotations = j.get<std::vector<Annotation>>();
    } catch (const json::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << '\n';
        annotations.clear();
    }
}

void AnnotationStore::save() {
    std::ofstream file(filepath);
    if (file.is_open()) {
        json j = annotations;
        file << j.dump(2);
    }
}

std::vector<Annotation> AnnotationStore::getByServer(const std::string& server) const {
    std::vector<Annotation> res;
    for (const auto& a : annotations) {
        if (a.server == server) {
            res.push_back(a);
        }
    }
    return res;
}

std::vector<Annotation> AnnotationStore::getManualByServer(const std::string& server) const {
    std::vector<Annotation> res;
    for (const auto& a : annotations) {
        if (a.server == server && a.source != "import") {
            res.push_back(a);
        }
    }
    return res;
}

std::vector<Annotation> AnnotationStore::getImportedByServer(const std::string& server) const {
    std::vector<Annotation> res;
    for (const auto& a : annotations) {
        if (a.server == server && a.source == "import") {
            res.push_back(a);
        }
    }
    return res;
}

void AnnotationStore::add(const Annotation& annotation) {
    annotations.push_back(annotation);
    save();
}

void AnnotationStore::update(const Annotation& updated) {
    for (auto& a : annotations) {
        if (a.id == updated.id) {
            a = updated;
            save();
            return;
        }
    }
}

void AnnotationStore::remove(const std::string& id) {
    auto it = annotations.begin();
    while (it != annotations.end()) {
        if (it->id == id) {
            it = annotations.erase(it);
        } else {
            ++it;
        }
    }
    save();
}

} // namespace models
} // namespace treasure
