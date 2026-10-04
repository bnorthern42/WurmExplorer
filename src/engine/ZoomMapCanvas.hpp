#pragma once

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QPoint>

#include "cv/Matcher.hpp"
#include <vector>
#include "../models/AnnotationStore.hpp"
#include "../models/DrawingStore.hpp"
#include "CanvasOverlays.hpp"

class ZoomMapCanvas : public QWidget {
    Q_OBJECT

signals:
    void shapeDrawn(const treasure::models::DrawingItem& item);
    void eraseRequested(const QString& id);

public:
    explicit ZoomMapCanvas(QWidget *parent = nullptr);
    ~ZoomMapCanvas() override;

    void loadImage(const QString& path);
    ::cv::Mat getMapMat() const;
    
    void centerOn(double x, double y);
    
    void setMarkers(const std::vector<treasure::cv::MatchResult>& markers);
    void clearMarkers();
    
    bool getRoi(::cv::Rect2i& out) const;
    void clearRoi();
    
    void setAnnotations(const std::vector<treasure::models::Annotation>& annotations);
    
    void setDrawings(const std::vector<treasure::models::DrawingObject>& drawings);
    
    void setOverlays(const treasure::ui::CanvasOverlays& overlays);
    
    void setInteractionMode(const QString& mode);
    void setDrawingColor(const QString& color);
    void setDrawingWidth(int width);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    double fitScaleForSize(int w, int h) const;
    void clampView();
    bool hitTest(const treasure::models::DrawingItem& item, QPointF pt, double tolerance) const;

    QImage mapImage;
    QPixmap mapPixmap;

    double scale;
    double viewX;
    double viewY;

    bool isDragging;
    QPoint lastMousePos;
    bool needsInitialCenter;
    
    std::vector<treasure::cv::MatchResult> m_markers;
    std::vector<treasure::models::Annotation> m_annotations;
    std::vector<treasure::models::DrawingObject> m_drawings;
    treasure::ui::CanvasOverlays m_overlays;
    
    QString m_interactionMode = "Drag"; // Drag, Line, Rectangle, Circle, Freehand
    QString m_drawingColor = "#ed8796";
    int m_drawingWidth = 2;
    
    bool isDrawing = false;
    QPointF drawStartPoint;
    QPointF drawCurrentPoint;
    std::vector<treasure::models::Point> freehandPoints;
    
    QRectF m_roi;
    bool m_roiActive = false;
    bool isDraggingRoi = false;
    QPointF roiStartPoint;
    QPointF m_cursorTilePos;
};
