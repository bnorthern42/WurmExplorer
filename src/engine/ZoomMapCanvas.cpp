#include "ZoomMapCanvas.hpp"

#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPainterPath>
#include <QPainterPathStroker>
#pragma push_macro("signals")
#undef signals
#include <vips/vips8>
#pragma pop_macro("signals")
#include <algorithm>
#include <cmath>

ZoomMapCanvas::ZoomMapCanvas(QWidget *parent)
    : QWidget(parent),
      scale(1.0),
      viewX(0.0),
      viewY(0.0),
      isDragging(false),
      needsInitialCenter(true) {
    setMouseTracking(true);
}

ZoomMapCanvas::~ZoomMapCanvas() = default;

void ZoomMapCanvas::loadImage(const QString& path) {
    try {
        vips::VImage img = vips::VImage::new_from_file(path.toStdString().c_str(),
            vips::VImage::option()->set("access", "sequential"));
        
        if (img.interpretation() != VIPS_INTERPRETATION_sRGB) {
            img = img.colourspace(VIPS_INTERPRETATION_sRGB);
        }
        if (img.bands() == 3) {
            img = img.bandjoin(255);
        }

        size_t size;
        void* buf = img.write_to_memory(&size);
        
        mapImage = QImage(static_cast<uchar*>(buf), img.width(), img.height(), 
                          img.width() * 4, QImage::Format_RGBA8888, 
                          [](void *info) { g_free(info); }, buf);

        if (!mapImage.isNull()) {
            mapPixmap = QPixmap::fromImage(mapImage);
            scale = fitScaleForSize(mapImage.width(), mapImage.height());
            viewX = 0;
            viewY = 0;
            needsInitialCenter = true;
            m_markers.clear();
            update();
        }
    } catch (vips::VError &e) {
        qWarning("Failed to load map: %s", e.what());
    }
}

::cv::Mat ZoomMapCanvas::getMapMat() const {
    if (mapImage.isNull()) return ::cv::Mat();
    
    // QImage is RGBA8888, cv::Mat expects BGR or BGRA.
    ::cv::Mat mat(mapImage.height(), mapImage.width(), CV_8UC4, (void*)mapImage.constBits(), mapImage.bytesPerLine());
    ::cv::Mat bgr;
    ::cv::cvtColor(mat, bgr, ::cv::COLOR_RGBA2BGR);
    return bgr;
}

void ZoomMapCanvas::centerOn(double x, double y) {
    if (mapImage.isNull()) return;
    
    // Ensure we are zoomed in enough to see the detail
    if (scale < 1.0) {
        scale = 1.0;
    }
    
    double vw = width() / scale;
    double vh = height() / scale;
    
    viewX = x - (vw / 2.0);
    viewY = y - (vh / 2.0);
    
    needsInitialCenter = false;
    clampView();
    update();
}

double ZoomMapCanvas::fitScaleForSize(int w, int h) const {
    if (w <= 0 || h <= 0) return 1.0;
    double sx = static_cast<double>(width()) / w;
    double sy = static_cast<double>(height()) / h;
    return std::min(sx, sy);
}

void ZoomMapCanvas::clampView() {
    if (mapImage.isNull()) return;

    double vw = width() / scale;
    double vh = height() / scale;

    viewX = std::max(0.0, std::min(viewX, static_cast<double>(mapImage.width()) - vw));
    viewY = std::max(0.0, std::min(viewY, static_cast<double>(mapImage.height()) - vh));
}

bool ZoomMapCanvas::hitTest(const treasure::models::DrawingItem& item, QPointF pt, double tolerance) const {
    if (item.points.empty()) return false;
    
    QPainterPath path;
    path.moveTo(item.points[0].x, item.points[0].y);
    
    if (item.tool == "Line" || item.tool == "Freehand" || item.tool == "Highlighter") {
        for (size_t i = 1; i < item.points.size(); ++i) {
            path.lineTo(item.points[i].x, item.points[i].y);
        }
    } else if (item.tool == "Rectangle") {
        if (item.points.size() >= 2) {
            path.addRect(QRectF(QPointF(item.points[0].x, item.points[0].y), QPointF(item.points[1].x, item.points[1].y)).normalized());
        }
    } else if (item.tool == "Circle") {
        if (item.points.size() >= 2) {
            double r = std::hypot(item.points[1].x - item.points[0].x, item.points[1].y - item.points[0].y);
            path.addEllipse(QPointF(item.points[0].x, item.points[0].y), r, r);
        }
    }
    
    QPainterPathStroker stroker;
    stroker.setWidth(std::max(10.0, static_cast<double>(item.width) + tolerance));
    QPainterPath strokePath = stroker.createStroke(path);
    
    return strokePath.contains(pt);
}

void ZoomMapCanvas::setMarkers(const std::vector<treasure::cv::MatchResult>& markers) {
    m_markers = markers;
    update();
}

void ZoomMapCanvas::setAnnotations(const std::vector<treasure::models::Annotation>& annotations) {
    m_annotations = annotations;
    update();
}

void ZoomMapCanvas::setOverlays(const treasure::ui::CanvasOverlays& overlays) {
    m_overlays = overlays;
    update();
}

void ZoomMapCanvas::setDrawings(const std::vector<treasure::models::DrawingObject>& drawings) {
    m_drawings = drawings;
    update();
}

void ZoomMapCanvas::setInteractionMode(const QString& mode) {
    m_interactionMode = mode;
    isDrawing = false;
    freehandPoints.clear();
    
    if (m_interactionMode == "Drag") {
        setCursor(Qt::OpenHandCursor);
    } else {
        setCursor(Qt::CrossCursor);
    }
}

void ZoomMapCanvas::setDrawingColor(const QString& color) {
    m_drawingColor = color;
}

void ZoomMapCanvas::setDrawingWidth(int width) {
    m_drawingWidth = width;
}

void ZoomMapCanvas::clearMarkers() {
    m_markers.clear();
    update();
}

bool ZoomMapCanvas::getRoi(::cv::Rect2i& out) const {
    if (m_roiActive) {
        // Ensure width and height are positive
        QRectF normRoi = m_roi.normalized();
        out = ::cv::Rect2i(normRoi.x(), normRoi.y(), normRoi.width(), normRoi.height());
        return true;
    }
    return false;
}

void ZoomMapCanvas::clearRoi() {
    m_roiActive = false;
    update();
}

void ZoomMapCanvas::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#18181b")); // Dark Slate Base

    if (mapPixmap.isNull()) {
        painter.setPen(QColor("#a1a1aa"));
        painter.drawText(rect(), Qt::AlignCenter, "No Map Loaded");
        return;
    }

    if (needsInitialCenter) {
        double fit = fitScaleForSize(mapPixmap.width(), mapPixmap.height());
        scale = std::max(1e-9, fit);
        viewX = (mapPixmap.width() - (width() / scale)) / 2.0;
        viewY = (mapPixmap.height() - (height() / scale)) / 2.0;
        clampView();
        needsInitialCenter = false;
    }

    double vw = width() / scale;
    double vh = height() / scale;

    QRectF targetRect(0, 0, width(), height());
    QRectF sourceRect(viewX, viewY, vw, vh);

    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawPixmap(targetRect, mapPixmap, sourceRect);

    // Draw markers
    if (!m_markers.empty()) {
        painter.setRenderHint(QPainter::Antialiasing);
        for (size_t i = 0; i < m_markers.size(); ++i) {
            const auto& marker = m_markers[i];
            
            // Map the matched coordinate to the screen view
            double cx = (marker.centerPx.x - viewX) * scale;
            double cy = (marker.centerPx.y - viewY) * scale;
            
            // Skip if completely out of view
            if (cx < -50 || cy < -50 || cx > width() + 50 || cy > height() + 50) continue;
            
            // For the top match (i=0), draw a highly visible crosshair + circle
            // For subsequent matches, draw smaller, dimmer markers
            if (i == 0) {
                QPen pen(QColor("#ed8796"), 3); // Red (Catppuccin Macchiato)
                painter.setPen(pen);
                
                int radius = 20;
                painter.drawEllipse(QPointF(cx, cy), radius, radius);
                
                // Crosshair lines
                painter.drawLine(QPointF(cx - radius - 10, cy), QPointF(cx + radius + 10, cy));
                painter.drawLine(QPointF(cx, cy - radius - 10), QPointF(cx, cy + radius + 10));
                
                // Label
                painter.setPen(QColor("#f4f4f5"));
                painter.drawText(QPointF(cx + radius + 5, cy - radius - 5), QString("Match %1%").arg(static_cast<int>(marker.score * 100)));
            } else {
                QPen pen(QColor("#04b97f"), 2); // Emerald
                pen.setStyle(Qt::DashLine);
                painter.setPen(pen);
                
                int radius = 10;
                painter.drawEllipse(QPointF(cx, cy), radius, radius);
            }
        }
    }
    
    // Draw annotations
    if (!m_annotations.empty()) {
        painter.setRenderHint(QPainter::Antialiasing);
        for (const auto& a : m_annotations) {
            if (a.points.empty()) continue;
            
            QColor strokeColor("#f5c2e7"); // Pink default
            QColor fillColor("#f5c2e7");
            QString icon = "place";
            
            if (a.type == "road") { strokeColor = QColor("#8bd5ca"); fillColor = Qt::transparent; icon = ""; }
            else if (a.type == "deed") { strokeColor = QColor("#c6a0f6"); fillColor = QColor(198, 160, 246, 50); icon = "home"; }
            else if (a.type == "bridge") { strokeColor = QColor("#eed49f"); fillColor = Qt::transparent; icon = "architecture"; }
            else if (a.type == "guard") { strokeColor = QColor("#ed8796"); fillColor = Qt::transparent; icon = "security"; }
            
            QPen pen(strokeColor, 2);
            painter.setPen(pen);
            painter.setBrush(fillColor);
            
            QPolygonF poly;
            for (const auto& p : a.points) {
                double px = (p.x - viewX) * scale;
                double py = (p.y - viewY) * scale;
                poly << QPointF(px, py);
            }
            
            if (a.points.size() == 1) {
                double cx = (a.points[0].x - viewX) * scale;
                double cy = (a.points[0].y - viewY) * scale;
                
                if (scale < 0.6 && !icon.isEmpty()) {
                    painter.setPen(QColor("#f4f4f5"));
                    QFont f("Material Icons");
                    f.setPointSize(12);
                    painter.setFont(f);
                    painter.drawText(QPointF(cx - 5, cy + 5), icon);
                } else {
                    painter.drawEllipse(QPointF(cx, cy), 5, 5);
                    if (scale >= 0.6) {
                        painter.setPen(QColor("#f4f4f5"));
                        painter.drawText(QPointF(cx + 10, cy - 10), QString::fromStdString(a.name));
                    }
                }
            } else {
                if (a.type == "deed") {
                    painter.drawPolygon(poly);
                } else {
                    painter.drawPolyline(poly);
                }
                
                double cx = (a.points[0].x - viewX) * scale;
                double cy = (a.points[0].y - viewY) * scale;
                
                if (scale < 0.6 && !icon.isEmpty() && a.type == "deed") {
                    painter.setPen(QColor("#f4f4f5"));
                    QFont f("Material Icons");
                    f.setPointSize(12);
                    painter.setFont(f);
                    painter.drawText(QPointF(cx - 5, cy + 5), icon);
                } else if (scale >= 0.6) {
                    painter.setPen(QColor("#f4f4f5"));
                    painter.drawText(QPointF(cx + 10, cy - 10), QString::fromStdString(a.name));
                }
            }
        }
    }
    
    // Draw persisted drawings
    for (const auto& obj : m_drawings) {
        if (!obj.visible) continue;
        for (const auto& item : obj.items) {
            if (!item.visible || item.points.empty()) continue;
            
            QColor color(QString::fromStdString(item.color));
            if (!color.isValid()) color = QColor("#ed8796"); // Red default
            if (item.tool == "Highlighter") color.setAlpha(128);
            
            QPen pen(color, item.width);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            
            if (item.tool == "Line") {
                if (item.points.size() >= 2) {
                    painter.drawLine(
                        (item.points[0].x - viewX) * scale, (item.points[0].y - viewY) * scale,
                        (item.points[1].x - viewX) * scale, (item.points[1].y - viewY) * scale
                    );
                }
            } else if (item.tool == "Rectangle") {
                if (item.points.size() >= 2) {
                    double x1 = (item.points[0].x - viewX) * scale;
                    double y1 = (item.points[0].y - viewY) * scale;
                    double x2 = (item.points[1].x - viewX) * scale;
                    double y2 = (item.points[1].y - viewY) * scale;
                    painter.drawRect(QRectF(QPointF(x1, y1), QPointF(x2, y2)).normalized());
                }
            } else if (item.tool == "Circle") {
                if (item.points.size() >= 2) {
                    double x1 = (item.points[0].x - viewX) * scale;
                    double y1 = (item.points[0].y - viewY) * scale;
                    double x2 = (item.points[1].x - viewX) * scale;
                    double y2 = (item.points[1].y - viewY) * scale;
                    double r = std::hypot(x2 - x1, y2 - y1);
                    painter.drawEllipse(QPointF(x1, y1), r, r);
                }
            } else if (item.tool == "Freehand" || item.tool == "Highlighter") {
                QPolygonF poly;
                for (const auto& p : item.points) {
                    poly << QPointF((p.x - viewX) * scale, (p.y - viewY) * scale);
                }
                painter.drawPolyline(poly);
            }
        }
    }
    
    // Draw active in-progress shape
    if (isDrawing && m_interactionMode != "Drag" && m_interactionMode != "Eraser") {
        QColor drawColor(m_drawingColor);
        if (!drawColor.isValid()) drawColor = QColor("#ed8796");
        if (m_interactionMode == "Highlighter") drawColor.setAlpha(128);
        
        QPen pen(drawColor, m_drawingWidth);
        if (m_interactionMode == "Measure") {
            pen.setStyle(Qt::DashLine);
        }
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        
        double sx = (drawStartPoint.x() - viewX) * scale;
        double sy = (drawStartPoint.y() - viewY) * scale;
        double cx = (drawCurrentPoint.x() - viewX) * scale;
        double cy = (drawCurrentPoint.y() - viewY) * scale;
        
        if (m_interactionMode == "Line" || m_interactionMode == "Measure") {
            painter.drawLine(QPointF(sx, sy), QPointF(cx, cy));
            
            if (m_interactionMode == "Measure") {
                double dist = std::hypot(drawCurrentPoint.x() - drawStartPoint.x(), drawCurrentPoint.y() - drawStartPoint.y());
                painter.setPen(QColor("#f4f4f5"));
                QString text = QString("%1 px").arg(static_cast<int>(dist));
                painter.drawText(QPointF((sx + cx) / 2 + 10, (sy + cy) / 2 - 10), text);
            }
        } else if (m_interactionMode == "Rectangle") {
            painter.drawRect(QRectF(QPointF(sx, sy), QPointF(cx, cy)).normalized());
        } else if (m_interactionMode == "Circle") {
            double r = std::hypot(cx - sx, cy - sy);
            painter.drawEllipse(QPointF(sx, sy), r, r);
        } else if (m_interactionMode == "Freehand" || m_interactionMode == "Highlighter") {
            QPolygonF poly;
            for (const auto& p : freehandPoints) {
                poly << QPointF((p.x - viewX) * scale, (p.y - viewY) * scale);
            }
            painter.drawPolyline(poly);
        }
    }
    
    // Draw ROI
    if (m_roiActive || isDraggingRoi) {
        painter.setPen(QPen(QColor("#04b97f"), 2, Qt::DashLine)); // Emerald dashed
        QColor fillColor("#04b97f");
        fillColor.setAlpha(45);
        painter.setBrush(fillColor);
        
        QRectF activeRoi = isDraggingRoi ? QRectF(roiStartPoint, drawCurrentPoint).normalized() : m_roi;
        
        double sx = (activeRoi.x() - viewX) * scale;
        double sy = (activeRoi.y() - viewY) * scale;
        double sw = activeRoi.width() * scale;
        double sh = activeRoi.height() * scale;
        
        painter.drawRect(QRectF(sx, sy, sw, sh));
    }

    // Floating HUD Overlay (Dashboard Aesthetic)
    if (!mapPixmap.isNull()) {
        painter.setRenderHint(QPainter::Antialiasing, true);
        QRect hudRect(14, height() - 42, 200, 28);
        painter.setPen(QPen(QColor("#383a42"), 1));
        painter.setBrush(QColor(24, 24, 27, 220));
        painter.drawRoundedRect(hudRect, 6, 6);
        
        int tx = std::clamp(static_cast<int>(m_cursorTilePos.x()), 0, mapPixmap.width());
        int ty = std::clamp(static_cast<int>(m_cursorTilePos.y()), 0, mapPixmap.height());
        
        QFont hFont = painter.font();
        hFont.setPointSize(9);
        hFont.setBold(true);
        painter.setFont(hFont);
        painter.setPen(QColor("#37efba"));
        painter.drawText(hudRect.adjusted(10, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft,
            QString("X: %1  Y: %2").arg(tx).arg(ty));
        painter.setPen(QColor("#a1a1aa"));
        painter.drawText(hudRect.adjusted(0, 0, -10, 0), Qt::AlignVCenter | Qt::AlignRight,
            QString("%1%").arg(static_cast<int>(scale * 100)));

        QRect modeRect(width() - 114, height() - 42, 100, 28);
        painter.setPen(QPen(QColor("#383a42"), 1));
        painter.drawRoundedRect(modeRect, 6, 6);
        painter.setPen(QColor("#04b97f"));
        painter.drawText(modeRect.adjusted(8, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft, "●");
        painter.setPen(QColor("#f4f4f5"));
        painter.drawText(modeRect.adjusted(20, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft, m_interactionMode);
    }
}

// Mouse and wheel event handling methods (wheelEvent, mousePressEvent,
// mouseMoveEvent, mouseReleaseEvent) are implemented in ZoomMapCanvasEvents.cpp
// to ensure file length remains strictly under 500 lines.

