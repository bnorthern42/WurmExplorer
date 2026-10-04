#include "ClusterLayout.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <queue>
#include <set>
#include <QPainter>
#include <QPen>
#include <QColor>
#include <QImageReader>

namespace treasure {
namespace models {

Direction oppositeDirection(Direction dir) {
    if (dir == Direction::North) return Direction::South;
    if (dir == Direction::South) return Direction::North;
    if (dir == Direction::East) return Direction::West;
    return Direction::East;
}

std::string directionToString(Direction dir) {
    if (dir == Direction::North) return "north";
    if (dir == Direction::East) return "east";
    if (dir == Direction::South) return "south";
    return "west";
}

std::optional<Direction> stringToDirection(const std::string& str) {
    if (str == "north") return Direction::North;
    if (str == "east") return Direction::East;
    if (str == "south") return Direction::South;
    if (str == "west") return Direction::West;
    return std::nullopt;
}

void ClusterGraph::addEdge(const std::string& from, const std::string& to, Direction dir) {
    auto addUnique = [](std::vector<ClusterEdge>& edges, const std::string& target, Direction d) {
        for (const auto& e : edges) { if (e.toServer == target && e.direction == d) return; }
        edges.push_back(ClusterEdge{target, d});
    };
    addUnique(adj[from], to, dir);
    addUnique(adj[to], from, oppositeDirection(dir));
}

const std::vector<ClusterEdge>& ClusterGraph::getNeighbors(const std::string& server) const {
    static const std::vector<ClusterEdge> empty;
    auto it = adj.find(server);
    return (it != adj.end()) ? it->second : empty;
}

bool ClusterGraph::hasServer(const std::string& server) const {
    return adj.find(server) != adj.end();
}

const std::map<std::string, std::vector<ClusterEdge>>& ClusterGraph::getAdjacencyList() const {
    return adj;
}

std::vector<std::string> ClusterGraph::findPath(const std::string& start, const std::string& goal) const {
    if (start.empty() || goal.empty()) return {};
    if (adj.find(start) == adj.end() && start != goal) return {};
    if (start == goal) return {start};

    std::queue<std::string> q;
    std::map<std::string, std::string> parent;
    std::set<std::string> visited;

    q.push(start);
    visited.insert(start);

    bool found = false;
    while (!q.empty()) {
        std::string current = q.front();
        q.pop();

        if (current == goal) {
            found = true;
            break;
        }

        auto it = adj.find(current);
        if (it != adj.end()) {
            for (const auto& edge : it->second) {
                if (visited.find(edge.toServer) == visited.end()) {
                    visited.insert(edge.toServer);
                    parent[edge.toServer] = current;
                    q.push(edge.toServer);
                }
            }
        }
    }

    if (!found) return {};

    std::vector<std::string> path;
    for (std::string curr = goal; curr != start; curr = parent[curr]) {
        path.push_back(curr);
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}

static std::map<std::string, ClusterGraph> initClusterGraphs() {
    std::map<std::string, ClusterGraph> clusterGraphs;

    // Epic cluster routing: set Elevation as central node.
    // Bidirectional connections: Elevation to Desertion (West), Serenity (East), and Affliction (South).
    clusterGraphs["Epic"].addEdge("Elevation", "Desertion", Direction::West);
    clusterGraphs["Epic"].addEdge("Elevation", "Serenity", Direction::East);
    clusterGraphs["Epic"].addEdge("Elevation", "Affliction", Direction::South);

    // Northern cluster routing: set Harmony as central node.
    // Bidirectional connections: Harmony to Cadence (West), Defiance (East), and Melody (South).
    clusterGraphs["Northern"].addEdge("Harmony", "Cadence", Direction::West);
    clusterGraphs["Northern"].addEdge("Harmony", "Defiance", Direction::East);
    clusterGraphs["Northern"].addEdge("Harmony", "Melody", Direction::South);

    // Southern cluster routing: preserve existing connections
    clusterGraphs["Southern"].addEdge("Independence", "Deliverance", Direction::South);
    clusterGraphs["Southern"].addEdge("Deliverance", "Exodus", Direction::South);
    clusterGraphs["Southern"].addEdge("Exodus", "Celebration", Direction::South);
    clusterGraphs["Southern"].addEdge("Independence", "Xanadu", Direction::East);
    clusterGraphs["Southern"].addEdge("Deliverance", "Xanadu", Direction::East);
    clusterGraphs["Southern"].addEdge("Exodus", "Xanadu", Direction::East);
    clusterGraphs["Southern"].addEdge("Celebration", "Xanadu", Direction::East);
    clusterGraphs["Southern"].addEdge("Xanadu", "Pristine", Direction::East);
    clusterGraphs["Southern"].addEdge("Xanadu", "Release", Direction::East);
    clusterGraphs["Southern"].addEdge("Chaos", "Independence", Direction::East);

    return clusterGraphs;
}

static const std::map<std::string, ClusterGraph>& getClusterGraphs() {
    static const std::map<std::string, ClusterGraph> s_graphs = initClusterGraphs();
    return s_graphs;
}

const ClusterGraph& SailingLogic::getClusterGraph(const std::string& clusterName) {
    const auto& graphs = getClusterGraphs();
    auto it = graphs.find(clusterName);
    if (it != graphs.end()) return it->second;
    static const ClusterGraph emptyGraph;
    return emptyGraph;
}

std::vector<std::string> SailingLogic::findRoute(const std::string& clusterName, const std::string& sourceServer, const std::string& destServer) {
    return getClusterGraph(clusterName).findPath(sourceServer, destServer);
}

const std::map<std::string, std::string> OPPOSITE_EDGE = {
    {"north", "south"},
    {"south", "north"},
    {"east", "west"},
    {"west", "east"}
};

// Global cluster tile coordinates. Sized so contiguous edges line up by global position.
const std::map<std::string, std::pair<float, float>> SOUTHERN_CLUSTER_LAYOUT = {
    {"Chaos", {0.0f, 3072.0f}},
    {"Independence", {4096.0f, 0.0f}},
    {"Deliverance", {6144.0f, 4096.0f}},
    {"Exodus", {6144.0f, 6144.0f}},
    {"Celebration", {6144.0f, 8192.0f}},
    {"Xanadu", {8192.0f, 2048.0f}},
    {"Pristine", {16384.0f, 3072.0f}},
    {"Release", {16384.0f, 7168.0f}}
};

const std::map<std::string, std::pair<float, float>> NORTHERN_CLUSTER_LAYOUT = {
    {"Cadence", {0.0f, 0.0f}},
    {"Harmony", {4096.0f, 0.0f}},
    {"Defiance", {8192.0f, 0.0f}},
    {"Melody", {4096.0f, 4096.0f}}
};

const std::map<std::string, std::pair<float, float>> EPIC_CLUSTER_LAYOUT = {
    {"Desertion", {0.0f, 0.0f}},
    {"Elevation", {2048.0f, 0.0f}},
    {"Serenity", {4096.0f, 0.0f}},
    {"Affliction", {2048.0f, 2048.0f}}
};

ClusterLayout SailingLogic::buildLayout(const std::map<std::string, int>& serverSizes, const std::string& clusterName) {
    ClusterLayout layout;
    layout.name = clusterName.empty() ? "Southern" : clusterName;
    
    const std::map<std::string, std::pair<float, float>>* baseLayout = &SOUTHERN_CLUSTER_LAYOUT;
    if (layout.name.find("Northern") != std::string::npos || 
        serverSizes.find("Cadence") != serverSizes.end() ||
        serverSizes.find("Harmony") != serverSizes.end()) {
        baseLayout = &NORTHERN_CLUSTER_LAYOUT;
    } else if (layout.name.find("Epic") != std::string::npos ||
               serverSizes.find("Elevation") != serverSizes.end() ||
               serverSizes.find("Affliction") != serverSizes.end()) {
        baseLayout = &EPIC_CLUSTER_LAYOUT;
    }
    
    float maxX = 1.0f;
    float maxY = 1.0f;
    
    for (const auto& pair : *baseLayout) {
        if (serverSizes.find(pair.first) == serverSizes.end()) continue;
        
        ClusterServer server;
        server.name = pair.first;
        server.size_tiles = serverSizes.at(pair.first);
        server.x0 = pair.second.first;
        server.y0 = pair.second.second;
        
        layout.servers[server.name] = server;
        if (server.x1() > maxX) maxX = server.x1();
        if (server.y1() > maxY) maxY = server.y1();
    }
    
    // Add any remaining servers not in the pre-defined layout
    float fallbackX = maxX;
    for (const auto& [name, size] : serverSizes) {
        if (layout.servers.find(name) != layout.servers.end()) continue;
        ClusterServer server;
        server.name = name;
        server.size_tiles = size;
        server.x0 = fallbackX;
        server.y0 = 0.0f;
        layout.servers[server.name] = server;
        fallbackX += size;
        if (server.x1() > maxX) maxX = server.x1();
        if (server.y1() > maxY) maxY = server.y1();
    }
    
    layout.width_tiles = maxX;
    layout.height_tiles = maxY;
    layout.graph = getClusterGraph(layout.name);
    return layout;
}

std::pair<float, float> SailingLogic::globalToImagePx(float x, float y, const ClusterRenderState& render) {
    return {
        render.margin_px + x * render.scale_px_per_tile,
        render.margin_px + y * render.scale_px_per_tile
    };
}

std::pair<float, float> SailingLogic::imagePxToGlobal(float x, float y, const ClusterRenderState& render) {
    float scale = std::max(1e-9f, render.scale_px_per_tile);
    return {
        (x - render.margin_px) / scale,
        (y - render.margin_px) / scale
    };
}

std::pair<float, float> SailingLogic::globalToServerTile(const ClusterServer& server, float gx, float gy) {
    return {gx - server.x0, gy - server.y0};
}

std::pair<float, float> SailingLogic::serverTileToGlobal(const ClusterServer& server, float tx, float ty) {
    return {server.x0 + tx, server.y0 + ty};
}

std::optional<EdgePick> SailingLogic::pickServerEdge(const ClusterLayout& layout, float gx, float gy, float tolerance_tiles) {
    std::optional<EdgePick> best = std::nullopt;
    float best_dist = 1e9f;
    
    for (const auto& pair : layout.servers) {
        const auto& server = pair.second;
        if (!(server.x0 - tolerance_tiles <= gx && gx <= server.x1() + tolerance_tiles &&
              server.y0 - tolerance_tiles <= gy && gy <= server.y1() + tolerance_tiles)) {
            continue;
        }
        
        struct Candidate {
            std::string edge;
            float dist;
            float norm;
            float tx;
            float ty;
            float global_pt_x;
            float global_pt_y;
        };
        
        std::vector<Candidate> candidates;
        
        if (server.x0 <= gx && gx <= server.x1()) {
            float tx = gx - server.x0;
            float size = std::max(1.0f, (float)server.size_tiles);
            candidates.push_back({"north", std::abs(gy - server.y0), tx / size, tx, 0.0f, gx, server.y0});
            candidates.push_back({"south", std::abs(gy - server.y1()), tx / size, tx, (float)server.size_tiles, gx, server.y1()});
        }
        if (server.y0 <= gy && gy <= server.y1()) {
            float ty = gy - server.y0;
            float size = std::max(1.0f, (float)server.size_tiles);
            candidates.push_back({"west", std::abs(gx - server.x0), ty / size, 0.0f, ty, server.x0, gy});
            candidates.push_back({"east", std::abs(gx - server.x1()), ty / size, (float)server.size_tiles, ty, server.x1(), gy});
        }
        
        for (const auto& c : candidates) {
            if (c.dist <= tolerance_tiles && c.dist < best_dist) {
                best_dist = c.dist;
                best = EdgePick{
                    server.name,
                    c.edge,
                    std::max(0.0f, std::min(1.0f, c.norm)),
                    c.tx, c.ty,
                    c.global_pt_x, c.global_pt_y
                };
            }
        }
    }
    
    return best;
}

static std::pair<float, float> _normToTile(const ClusterServer& server, const std::string& edge, float norm) {
    float t = std::max(0.0f, std::min(1.0f, norm));
    float size = server.size_tiles;
    if (edge == "north") return {t * size, 0.0f};
    if (edge == "south") return {t * size, size};
    if (edge == "west") return {0.0f, t * size};
    return {size, t * size}; // east
}

static std::pair<float, float> _tileToGlobal(const ClusterServer& server, const std::string& edge, float norm) {
    auto t = _normToTile(server, edge, norm);
    return SailingLogic::serverTileToGlobal(server, t.first, t.second);
}

std::optional<SailingResult> SailingLogic::resolvePlotCourse(const ClusterLayout& layout, const std::string& source_server, const std::string& source_edge, float source_norm, const std::string& dest_server) {
    if (layout.servers.find(source_server) == layout.servers.end()) return std::nullopt;
    if (layout.servers.find(dest_server) == layout.servers.end()) return std::nullopt;
    if (OPPOSITE_EDGE.find(source_edge) == OPPOSITE_EDGE.end()) return std::nullopt;
    
    const auto& src = layout.servers.at(source_server);
    const auto& dst = layout.servers.at(dest_server);
    std::string dest_edge = OPPOSITE_EDGE.at(source_edge);
    
    auto src_tile = _normToTile(src, source_edge, source_norm);
    auto dst_tile = _normToTile(dst, dest_edge, source_norm);
    
    auto src_global = serverTileToGlobal(src, src_tile.first, src_tile.second);
    auto dst_global = serverTileToGlobal(dst, dst_tile.first, dst_tile.second);
    
    auto path = layout.graph.findPath(source_server, dest_server);
    
    return SailingResult{
        "plot_course",
        src.name, source_edge, std::max(0.0f, std::min(1.0f, source_norm)),
        src_tile.first, src_tile.second,
        dst.name, dest_edge, dst_tile.first, dst_tile.second,
        src_global.first, src_global.second,
        dst_global.first, dst_global.second,
        path
    };
}

std::optional<SailingResult> SailingLogic::resolveRegularCrossing(const ClusterLayout& layout, const std::string& source_server, const std::string& source_edge, float source_norm) {
    if (layout.servers.find(source_server) == layout.servers.end()) return std::nullopt;
    const auto& src = layout.servers.at(source_server);
    auto src_global = _tileToGlobal(src, source_edge, source_norm);
    
    float eps = 1e-4f;
    const ClusterServer* chosen = nullptr;
    std::string dest_edge;
    
    for (const auto& [_, srv] : layout.servers) {
        if (srv.name == src.name) continue;
        if (source_edge == "east" && std::abs(srv.x0 - src.x1()) <= eps && srv.y0 - eps <= src_global.second && src_global.second <= srv.y1() + eps) {
            chosen = &srv; dest_edge = "west"; break;
        } else if (source_edge == "west" && std::abs(srv.x1() - src.x0) <= eps && srv.y0 - eps <= src_global.second && src_global.second <= srv.y1() + eps) {
            chosen = &srv; dest_edge = "east"; break;
        } else if (source_edge == "north" && std::abs(srv.y1() - src.y0) <= eps && srv.x0 - eps <= src_global.first && src_global.first <= srv.x1() + eps) {
            chosen = &srv; dest_edge = "south"; break;
        } else if (source_edge == "south" && std::abs(srv.y0 - src.y1()) <= eps && srv.x0 - eps <= src_global.first && src_global.first <= srv.x1() + eps) {
            chosen = &srv; dest_edge = "north"; break;
        }
    }
    
    if (!chosen || dest_edge.empty()) return std::nullopt;
    
    float dest_norm = 0.0f;
    if (dest_edge == "north" || dest_edge == "south") {
        dest_norm = (src_global.first - chosen->x0) / std::max(1.0f, (float)chosen->size_tiles);
    } else {
        dest_norm = (src_global.second - chosen->y0) / std::max(1.0f, (float)chosen->size_tiles);
    }
    
    dest_norm = std::max(0.0f, std::min(1.0f, dest_norm));
    auto dst_tile = _normToTile(*chosen, dest_edge, dest_norm);
    auto dst_global = serverTileToGlobal(*chosen, dst_tile.first, dst_tile.second);
    auto src_tile = _normToTile(src, source_edge, std::max(0.0f, std::min(1.0f, source_norm)));
    
    auto path = layout.graph.findPath(src.name, chosen->name);
    if (path.empty()) path = {src.name, chosen->name};
    
    return SailingResult{
        "regular",
        src.name, source_edge, std::max(0.0f, std::min(1.0f, source_norm)),
        src_tile.first, src_tile.second,
        chosen->name, dest_edge, dst_tile.first, dst_tile.second,
        src_global.first, src_global.second,
        dst_global.first, dst_global.second,
        path
    };
}

ClusterRenderState SailingLogic::renderClusterMap(const ClusterLayout& layout, const std::map<std::string, std::string>& mapImages, const std::string& mapType) {
    int margin_px = 80;
    
    float max_w = 1900.0f;
    float max_h = 1150.0f;
    float scale = std::min(max_w / std::max(1.0f, layout.width_tiles), max_h / std::max(1.0f, layout.height_tiles));
    
    int width_px = static_cast<int>(std::round(layout.width_tiles * scale)) + margin_px * 2;
    int height_px = static_cast<int>(std::round(layout.height_tiles * scale)) + margin_px * 2;
    
    QImage canvas(std::max(1, width_px), std::max(1, height_px), QImage::Format_RGB32);
    canvas.fill(QColor(27, 28, 31)); // match python's background
    
    QPainter p(&canvas);
    p.fillRect(0, 0, width_px - 1, height_px - 1, QColor(28, 29, 32));
    
    p.setPen(QColor(220, 220, 220));
    p.drawText(18, 24, QString("%1 - %2").arg(QString::fromStdString(layout.name), QString::fromStdString(mapType)));
    
    std::vector<std::string> serverOrder = {
        "Independence", "Deliverance", "Exodus", "Celebration", 
        "Pristine", "Release", "Xanadu", "Chaos",
        "Harmony", "Melody", "Cadence", "Defiance",
        "Elevation", "Desertion", "Serenity", "Affliction"
    };
    for (const auto& [name, _] : layout.servers) {
        if (std::find(serverOrder.begin(), serverOrder.end(), name) == serverOrder.end()) {
            serverOrder.push_back(name);
        }
    }
    
    for (const auto& name : serverOrder) {
        if (layout.servers.find(name) == layout.servers.end()) continue;
        const auto& srv = layout.servers.at(name);
        
        int x0 = static_cast<int>(std::round(margin_px + srv.x0 * scale));
        int y0 = static_cast<int>(std::round(margin_px + srv.y0 * scale));
        int x1 = static_cast<int>(std::round(margin_px + srv.x1() * scale));
        int y1 = static_cast<int>(std::round(margin_px + srv.y1() * scale));
        
        bool imageDrawn = false;
        if (mapImages.find(name) != mapImages.end()) {
            QImageReader reader(QString::fromStdString(mapImages.at(name)));
            if (reader.canRead()) {
                QImage mapImg = reader.read();
                if (!mapImg.isNull()) {
                    QImage scaled = mapImg.scaled(std::max(1, x1 - x0), std::max(1, y1 - y0), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    p.drawImage(x0, y0, scaled);
                    imageDrawn = true;
                }
            }
        }
        
        if (!imageDrawn) {
            p.fillRect(x0, y0, std::max(1, x1 - x0), std::max(1, y1 - y0), QColor(65, 76, 89));
        }
        
        // border
        p.setPen(QPen(QColor(230, 230, 235), 2));
        p.setBrush(Qt::NoBrush);
        p.drawRect(x0, y0, std::max(1, x1 - x0), std::max(1, y1 - y0));
        
        // label
        QString label = QString("%1 - %2").arg(QString::fromStdString(name)).arg(srv.size_tiles);
        int text_w = label.length() * 8; // approx
        int text_h = 20;
        
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(20, 20, 22, 180));
        p.drawRoundedRect(x0 + 8, y0 + 8, text_w, text_h, 6, 6);
        
        p.setPen(QColor(245, 245, 245));
        p.drawText(x0 + 14, y0 + 22, label);
    }
    
    p.end();
    
    return ClusterRenderState{
        canvas,
        scale,
        margin_px,
        width_px,
        height_px
    };
}

} // namespace models
} // namespace treasure
