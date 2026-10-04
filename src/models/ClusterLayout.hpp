#pragma once

#include <string>
#include <map>
#include <vector>
#include <optional>
#include <QImage>

namespace treasure {
namespace models {

enum class Direction {
    North,
    East,
    South,
    West
};

Direction oppositeDirection(Direction dir);
std::string directionToString(Direction dir);
std::optional<Direction> stringToDirection(const std::string& str);

struct ClusterEdge {
    std::string toServer;
    Direction direction;
};

class ClusterGraph {
public:
    ClusterGraph() = default;

    void addEdge(const std::string& from, const std::string& to, Direction dir);
    const std::vector<ClusterEdge>& getNeighbors(const std::string& server) const;
    std::vector<std::string> findPath(const std::string& start, const std::string& goal) const;
    bool hasServer(const std::string& server) const;
    const std::map<std::string, std::vector<ClusterEdge>>& getAdjacencyList() const;

private:
    std::map<std::string, std::vector<ClusterEdge>> adj;
};

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
    ClusterGraph graph;
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

    std::vector<std::string> path;
};

class SailingLogic {
public:
    static ClusterLayout buildLayout(const std::map<std::string, int>& serverSizes, const std::string& clusterName = "Southern");
    
    static const ClusterGraph& getClusterGraph(const std::string& clusterName);
    static std::vector<std::string> findRoute(const std::string& clusterName, const std::string& sourceServer, const std::string& destServer);

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
