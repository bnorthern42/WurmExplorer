#pragma once

#include <QString>
#include <QColor>
#include <vector>
#include <utility>

namespace treasure {
namespace ui {

struct OverlayPolygon {
    std::vector<std::pair<float, float>> points;
    QColor lineColor;
    QColor fillColor;
    int width;
    QString pattern;
};

struct OverlayLine {
    std::vector<std::pair<float, float>> points;
    QColor color;
    int width;
    std::vector<qreal> dashPattern;
};

struct OverlayCircle {
    float cx;
    float cy;
    float radius;
    QColor color;
};

struct OverlayPoint {
    float px;
    float py;
    QColor color;
};

struct OverlayText {
    float tx;
    float ty;
    QString text;
    QColor color;
};

struct CanvasOverlays {
    std::vector<OverlayPolygon> polygons;
    std::vector<OverlayLine> lines;
    std::vector<OverlayCircle> circles;
    std::vector<OverlayPoint> points;
    std::vector<OverlayText> texts;
    
    void clear() {
        polygons.clear();
        lines.clear();
        circles.clear();
        points.clear();
        texts.clear();
    }
};

}
}
