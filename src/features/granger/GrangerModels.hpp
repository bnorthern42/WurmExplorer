#pragma once

#include <string>
#include <vector>
#include <cmath>
#include <nlohmann/json.hpp>

namespace granger {

struct Player {
    int id = 0;
    std::string name;
    int ah_skill = 0;
    int extra_slots = 0;

    int maxSlots() const {
        return 1 + static_cast<int>(std::floor(ah_skill / 10.0)) + extra_slots;
    }
};

struct Animal {
    int id = 0;
    std::string name;
    std::string type = "horse";
    std::vector<std::string> traits;
    int mother_id = 0;
    int father_id = 0;
    int cared_by = 0;
    std::string notes;
};

inline void to_json(nlohmann::json& j, const Player& p) {
    j = nlohmann::json{
        {"id", p.id},
        {"name", p.name},
        {"ah_skill", p.ah_skill},
        {"extra_slots", p.extra_slots}
    };
}

inline void from_json(const nlohmann::json& j, Player& p) {
    j.at("id").get_to(p.id);
    j.at("name").get_to(p.name);
    if (j.contains("ah_skill")) j.at("ah_skill").get_to(p.ah_skill);
    if (j.contains("extra_slots")) j.at("extra_slots").get_to(p.extra_slots);
}

inline void to_json(nlohmann::json& j, const Animal& a) {
    j = nlohmann::json{
        {"id", a.id},
        {"name", a.name},
        {"type", a.type},
        {"traits", a.traits},
        {"mother_id", a.mother_id},
        {"father_id", a.father_id},
        {"cared_by", a.cared_by},
        {"notes", a.notes}
    };
}

inline void from_json(const nlohmann::json& j, Animal& a) {
    j.at("id").get_to(a.id);
    j.at("name").get_to(a.name);
    if (j.contains("type")) j.at("type").get_to(a.type);
    if (j.contains("traits") && j["traits"].is_array()) {
        j.at("traits").get_to(a.traits);
    } else {
        a.traits.clear();
    }
    if (j.contains("mother_id")) j.at("mother_id").get_to(a.mother_id);
    if (j.contains("father_id")) j.at("father_id").get_to(a.father_id);
    if (j.contains("cared_by")) j.at("cared_by").get_to(a.cared_by);
    if (j.contains("notes")) j.at("notes").get_to(a.notes);
}

} // namespace granger
