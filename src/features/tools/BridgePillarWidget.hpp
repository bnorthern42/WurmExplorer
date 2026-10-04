#pragma once

#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include "BridgePillarCalculator.hpp"

namespace tools {

class PillarElevationCanvas : public QWidget {
    Q_OBJECT
public:
    explicit PillarElevationCanvas(QWidget* parent = nullptr);
    void setResult(const PillarResult& res);
    void updateGeometryForViewport(const QSize& viewportSize);

signals:
    void cornerHovered(int x, int y, int height);
    void tileHovered(int x, int y, int height);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    PillarResult m_result;
    int m_hoverX = -1;
    int m_hoverY = -1;
};

class BridgePillarWidget : public QWidget {
    Q_OBJECT
public:
    explicit BridgePillarWidget(QWidget* parent = nullptr);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void recalculate();
    void copyHeightmap();

private:
    void setupUi();
    void updateCanvasSize();

    QSpinBox* m_topWSpin = nullptr;
    QSpinBox* m_topLSpin = nullptr;
    QSpinBox* m_heightSpin = nullptr;
    QCheckBox* m_skillCheck = nullptr;
    QDoubleSpinBox* m_skillSpin = nullptr;
    QCheckBox* m_pvpCheck = nullptr;

    QLabel* m_totalDirtLabel = nullptr;
    QLabel* m_cratesLabel = nullptr;
    QLabel* m_radiusLabel = nullptr;
    QLabel* m_footprintLabel = nullptr;
    QLabel* m_hoverDetailLabel = nullptr;

    QScrollArea* m_scrollArea = nullptr;
    PillarElevationCanvas* m_canvas = nullptr;
    PillarResult m_lastResult;
};

} // namespace tools
