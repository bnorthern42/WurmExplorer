#pragma once

#include <string>
#include <map>
#include <vector>
#include <optional>
#include <QImage>

namespace treasure {
namespace models {

struct ClusterServer {
    std::string name;
    int size_tiles;
    float x0;
    float y0;

    float x1() const { return x0 + size_tiles; }
    float y1() const { return y0 + size_tiles; }
};

struct ClusterLayout {
    std::string name;
    std::map<std::string, ClusterServer> servers;
    float width_tiles;
    float height_tiles;
};

struct ClusterRenderState {
    QImage image;
    float scale_px_per_tile;
    int margin_px;
    int width_px;
    int height_px;
};

struct EdgePick {
    std::string server;
    std::string edge;
    float norm;
    float tile_x;
    float tile_y;
    float global_tile_x;
    float global_tile_y;
};

struct SailingResult {
    std::string mode;
    std::string source_server;
    std::string source_edge;
    float source_norm;
    float source_tile_x;
    float source_tile_y;
    
    std::string dest_server;
    std::string dest_edge;
    float dest_tile_x;
    float dest_tile_y;
    
    float source_global_tile_x;
    float source_global_tile_y;
    float dest_global_tile_x;
    float dest_global_tile_y;
};

class SailingLogic {
public:
    static ClusterLayout buildLayout(const std::map<std::string, int>& serverSizes);
    
    static std::pair<float, float> globalToImagePx(float x, float y, const ClusterRenderState& render);
    static std::pair<float, float> imagePxToGlobal(float x, float y, const ClusterRenderState& render);
    
    static std::pair<float, float> globalToServerTile(const ClusterServer& server, float gx, float gy);
    static std::pair<float, float> serverTileToGlobal(const ClusterServer& server, float tx, float ty);
    
    static std::optional<EdgePick> pickServerEdge(const ClusterLayout& layout, float gx, float gy, float tolerance_tiles = 220.0f);
    
    static std::optional<SailingResult> resolvePlotCourse(const ClusterLayout& layout, const std::string& source_server, const std::string& source_edge, float source_norm, const std::string& dest_server);
    
    static std::optional<SailingResult> resolveRegularCrossing(const ClusterLayout& layout, const std::string& source_server, const std::string& source_edge, float source_norm);

    static ClusterRenderState renderClusterMap(const ClusterLayout& layout, const std::map<std::string, std::string>& mapImages, const std::string& mapType);
};

} // namespace models
} // namespace treasure
