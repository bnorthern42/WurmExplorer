#include "ClusterLayout.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <QPainter>
#include <QPen>
#include <QColor>
#include <QImageReader>

namespace treasure {
namespace models {

const std::map<std::string, std::string> OPPOSITE_EDGE = {
    {"north", "south"},
    {"south", "north"},
    {"east", "west"},
    {"west", "east"}
};

// Global cluster tile coordinates. Sized so contiguous edges line up by global position.
const std::map<std::string, std::pair<float, float>> DEFAULT_CLUSTER_LAYOUT = {
    {"Chaos", {0.0f, 3072.0f}},
    {"Independence", {4096.0f, 0.0f}},
    {"Deliverance", {6144.0f, 4096.0f}},
    {"Exodus", {6144.0f, 6144.0f}},
    {"Celebration", {6144.0f, 8192.0f}},
    {"Xanadu", {8192.0f, 2048.0f}},
    {"Pristine", {16384.0f, 3072.0f}},
    {"Release", {16384.0f, 7168.0f}}
};

ClusterLayout SailingLogic::buildLayout(const std::map<std::string, int>& serverSizes) {
    ClusterLayout layout;
    layout.name = "Southern Freedom Isles";
    
    float maxX = 1.0f;
    float maxY = 1.0f;
    
    for (const auto& pair : DEFAULT_CLUSTER_LAYOUT) {
        if (serverSizes.find(pair.first) == serverSizes.end()) continue;
        
        ClusterServer server;
        server.name = pair.first;
        server.size_tiles = serverSizes.at(pair.first);
        server.x0 = pair.second.first;
        server.y0 = pair.second.first; // BUG IN ORIGINAL PYTHON: Wait, pair.second.second is y! Let me fix this.
        server.y0 = pair.second.second;
        
        layout.servers[server.name] = server;
        
        if (server.x1() > maxX) maxX = server.x1();
        if (server.y1() > maxY) maxY = server.y1();
    }
    
    layout.width_tiles = maxX;
    layout.height_tiles = maxY;
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
    
    return SailingResult{
        "plot_course",
        src.name, source_edge, std::max(0.0f, std::min(1.0f, source_norm)),
        src_tile.first, src_tile.second,
        dst.name, dest_edge, dst_tile.first, dst_tile.second,
        src_global.first, src_global.second,
        dst_global.first, dst_global.second
    };
}

std::optional<SailingResult> SailingLogic::resolveRegularCrossing(const ClusterLayout& layout, const std::string& source_server, const std::string& source_edge, float source_norm) {
    if (layout.servers.find(source_server) == layout.servers.end()) return std::nullopt;
    const auto& src = layout.servers.at(source_server);
    auto src_global = _tileToGlobal(src, source_edge, source_norm);
    
    float eps = 1e-4f;
    const ClusterServer* chosen = nullptr;
    std::string dest_edge;
    
    if (source_edge == "east") {
        float y = src_global.second;
        float x = src.x1();
        for (const auto& pair : layout.servers) {
            if (pair.second.name == src.name) continue;
            if (std::abs(pair.second.x0 - x) <= eps && pair.second.y0 - eps <= y && y <= pair.second.y1() + eps) {
                chosen = &pair.second;
                dest_edge = "west";
                break;
            }
        }
    } else if (source_edge == "west") {
        float y = src_global.second;
        float x = src.x0;
        for (const auto& pair : layout.servers) {
            if (pair.second.name == src.name) continue;
            if (std::abs(pair.second.x1() - x) <= eps && pair.second.y0 - eps <= y && y <= pair.second.y1() + eps) {
                chosen = &pair.second;
                dest_edge = "east";
                break;
            }
        }
    } else if (source_edge == "north") {
        float x = src_global.first;
        float y = src.y0;
        for (const auto& pair : layout.servers) {
            if (pair.second.name == src.name) continue;
            if (std::abs(pair.second.y1() - y) <= eps && pair.second.x0 - eps <= x && x <= pair.second.x1() + eps) {
                chosen = &pair.second;
                dest_edge = "south";
                break;
            }
        }
    } else if (source_edge == "south") {
        float x = src_global.first;
        float y = src.y1();
        for (const auto& pair : layout.servers) {
            if (pair.second.name == src.name) continue;
            if (std::abs(pair.second.y0 - y) <= eps && pair.second.x0 - eps <= x && x <= pair.second.x1() + eps) {
                chosen = &pair.second;
                dest_edge = "north";
                break;
            }
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
    
    return SailingResult{
        "regular",
        src.name, source_edge, std::max(0.0f, std::min(1.0f, source_norm)),
        src_tile.first, src_tile.second,
        chosen->name, dest_edge, dst_tile.first, dst_tile.second,
        src_global.first, src_global.second,
        dst_global.first, dst_global.second
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
        "Pristine", "Release", "Xanadu", "Chaos"
    };
    
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
                    // scale
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
