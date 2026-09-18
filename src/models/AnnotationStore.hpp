#pragma once

#include <string>
#include <vector>

// Forward declare for JSON
#include <nlohmann/json.hpp>

namespace treasure {
namespace models {

struct Point {
    float x;
    float y;
};

void to_json(nlohmann::json& j, const Point& p);
void from_json(const nlohmann::json& j, Point& p);

struct Annotation {
    std::string id;
    std::string server;
    std::string type;
    std::string status;
    std::string name;
    
    std::string notes = "";
    std::string tags = "";
    std::vector<Point> points;
    
    std::string source = "manual";
    std::string source_name = "";
    std::string source_key = "";
    std::string source_sheet = "";
    std::string category = "";
    std::string imported_kind = "";
    bool visible = true;
    
    std::string kingdom = "";
    int influence_radius = 0;
};

class AnnotationStore {
public:
    AnnotationStore(const std::string& filepath);
    
    void load();
    void save();
    
    std::vector<Annotation> getByServer(const std::string& server) const;
    std::vector<Annotation> getManualByServer(const std::string& server) const;
    std::vector<Annotation> getImportedByServer(const std::string& server) const;
    
    void add(const Annotation& annotation);
    void update(const Annotation& annotation);
    void remove(const std::string& id);

private:
    std::string filepath;
    std::vector<Annotation> annotations;
};

} // namespace models
} // namespace treasure
