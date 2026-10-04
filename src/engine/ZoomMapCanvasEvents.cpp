#include "ZoomMapCanvas.hpp"
#include <QWheelEvent>
#include <QMouseEvent>
#include <cmath>
#include <algorithm>

void ZoomMapCanvas::wheelEvent(QWheelEvent *event) {
    if (mapPixmap.isNull()) return;

    double zoomFactor = std::pow(1.0015, event->angleDelta().y());
    
    // Calculate mouse position in image coordinates
    double mx = viewX + event->position().x() / scale;
    double my = viewY + event->position().y() / scale;

    double fit = fitScaleForSize(mapPixmap.width(), mapPixmap.height());
    scale = std::clamp(scale * zoomFactor, fit, 24.0);

    // Adjust view to keep mouse position fixed
    viewX = mx - event->position().x() / scale;
    viewY = my - event->position().y() / scale;

    clampView();
    update();
}

void ZoomMapCanvas::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (m_interactionMode == "Drag") {
            isDragging = true;
            lastMousePos = event->pos();
            setCursor(Qt::ClosedHandCursor);
        } else {
            isDrawing = true;
            drawStartPoint = QPointF(viewX + event->pos().x() / scale, viewY + event->pos().y() / scale);
            drawCurrentPoint = drawStartPoint;
            
            if (m_interactionMode == "Select ROI") {
                isDraggingRoi = true;
                roiStartPoint = drawStartPoint;
                m_roiActive = false;
            } else if (m_interactionMode == "Freehand" || m_interactionMode == "Highlighter") {
                freehandPoints.clear();
                freehandPoints.push_back(treasure::models::Point{static_cast<float>(drawStartPoint.x()), static_cast<float>(drawStartPoint.y())});
            } else if (m_interactionMode == "Eraser") {
                double mapX = viewX + event->pos().x() / scale;
                double mapY = viewY + event->pos().y() / scale;
                QPointF pt(mapX, mapY);
                for (const auto& obj : m_drawings) {
                    if (!obj.visible) continue;
                    for (const auto& item : obj.items) {
                        if (hitTest(item, pt, 15.0 / scale)) {
                            emit eraseRequested(QString::fromStdString(obj.id));
                            isDrawing = false; // Prevent dragging over same object
                            return;
                        }
                    }
                }
            }
        }
    }
}

void ZoomMapCanvas::mouseMoveEvent(QMouseEvent *event) {
    if (!mapPixmap.isNull()) {
        m_cursorTilePos = QPointF(viewX + event->pos().x() / scale, viewY + event->pos().y() / scale);
    }
    
    if (isDragging && !mapPixmap.isNull()) {
        QPoint delta = event->pos() - lastMousePos;
        lastMousePos = event->pos();

        viewX -= delta.x() / scale;
        viewY -= delta.y() / scale;

        clampView();
        update();
    } else if (isDrawing && !mapPixmap.isNull()) {
        drawCurrentPoint = m_cursorTilePos;
        
        if (m_interactionMode == "Freehand" || m_interactionMode == "Highlighter") {
            freehandPoints.push_back(treasure::models::Point{static_cast<float>(drawCurrentPoint.x()), static_cast<float>(drawCurrentPoint.y())});
        } else if (m_interactionMode == "Eraser") {
            QPointF pt = drawCurrentPoint;
            for (const auto& obj : m_drawings) {
                if (!obj.visible) continue;
                for (const auto& item : obj.items) {
                    if (hitTest(item, pt, 15.0 / scale)) {
                        emit eraseRequested(QString::fromStdString(obj.id));
                        isDrawing = false;
                        return;
                    }
                }
            }
        }
        
        update();
    } else {
        update(); // Refresh floating coordinate HUD
    }
}

void ZoomMapCanvas::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (isDragging) {
            isDragging = false;
            setCursor(Qt::OpenHandCursor);
        } else if (isDrawing) {
            isDrawing = false;
            
            if (m_interactionMode == "Select ROI") {
                isDraggingRoi = false;
                m_roi = QRectF(roiStartPoint, drawCurrentPoint).normalized();
                m_roiActive = true;
                update();
                return;
            }
            
            if (m_interactionMode == "Measure") {
                update();
                return;
            }
            
            if (m_interactionMode == "Eraser") {
                // Do not persist eraser drags
                update();
                return;
            }
            
            treasure::models::DrawingItem item;
            item.tool = m_interactionMode.toStdString();
            item.color = m_drawingColor.toStdString();
            item.width = m_drawingWidth;
            
            if (m_interactionMode == "Freehand" || m_interactionMode == "Highlighter") {
                item.points = freehandPoints;
            } else if (m_interactionMode == "Line" || m_interactionMode == "Rectangle" || m_interactionMode == "Circle") {
                item.points.push_back(treasure::models::Point{static_cast<float>(drawStartPoint.x()), static_cast<float>(drawStartPoint.y())});
                item.points.push_back(treasure::models::Point{static_cast<float>(drawCurrentPoint.x()), static_cast<float>(drawCurrentPoint.y())});
            }
            
            if (!item.points.empty()) {
                emit shapeDrawn(item);
            }
        }
    }
}
