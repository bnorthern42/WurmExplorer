#include "BridgePillarWidget.hpp"
#include "../../ui/ThemeTokens.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPainter>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QApplication>
#include <QClipboard>
#include <QScrollArea>
#include <QEvent>
#include <QShowEvent>
#include <algorithm>

namespace tools {

using namespace treasure::ui;

// --- PillarElevationCanvas Implementation ---

PillarElevationCanvas::PillarElevationCanvas(QWidget* parent)
    : QWidget(parent) {
    setMouseTracking(true);
    setMinimumSize(280, 280);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void PillarElevationCanvas::setResult(const PillarResult& res) {
    m_result = res;
    m_hoverX = -1;
    m_hoverY = -1;
    update();
}

void PillarElevationCanvas::updateGeometryForViewport(const QSize& viewportSize) {
    if (m_result.cornerW <= 0 || m_result.cornerL <= 0) {
        int w = std::max(280, viewportSize.width());
        int h = std::max(280, viewportSize.height());
        setFixedSize(w, h);
        return;
    }

    double cellW = std::max(static_cast<double>(theme::CELL_MIN_WIDTH_PX),
                            viewportSize.width() > 0 ? static_cast<double>(viewportSize.width()) / m_result.cornerW : static_cast<double>(theme::CELL_MIN_WIDTH_PX));
    double cellH = std::max(static_cast<double>(theme::CELL_MIN_HEIGHT_PX),
                            viewportSize.height() > 0 ? static_cast<double>(viewportSize.height()) / m_result.cornerL : static_cast<double>(theme::CELL_MIN_HEIGHT_PX));

    int totalW = static_cast<int>(std::ceil(cellW * m_result.cornerW));
    int totalH = static_cast<int>(std::ceil(cellH * m_result.cornerL));
    setFixedSize(totalW, totalH);
    update();
}

void PillarElevationCanvas::mouseMoveEvent(QMouseEvent* event) {
    if (m_result.cornerW <= 0 || m_result.cornerL <= 0) return;

    double cellW = static_cast<double>(width()) / m_result.cornerW;
    double cellH = static_cast<double>(height()) / m_result.cornerL;

    int cx = static_cast<int>(event->position().x() / cellW);
    int cy = static_cast<int>(event->position().y() / cellH);

    if (cx >= 0 && cx < m_result.cornerW && cy >= 0 && cy < m_result.cornerL) {
        if (cx != m_hoverX || cy != m_hoverY) {
            m_hoverX = cx;
            m_hoverY = cy;
            int height = m_result.cornerGrid[cy][cx];
            int tier = BridgePillarCalculator::getElevationTier(height);
            emit cornerHovered(cx, cy, height);
            emit tileHovered(cx, cy, height);
            setToolTip(QString("Corner [%1, %2] | Height: %3 dirt (Tier %4)").arg(cx).arg(cy).arg(height).arg(tier));
            update();
        }
    } else {
        leaveEvent(nullptr);
    }
}

void PillarElevationCanvas::leaveEvent(QEvent*) {
    if (m_hoverX != -1 || m_hoverY != -1) {
        m_hoverX = -1;
        m_hoverY = -1;
        emit cornerHovered(-1, -1, 0);
        emit tileHovered(-1, -1, 0);
        setToolTip(QString());
        update();
    }
}

void PillarElevationCanvas::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    painter.fillRect(rect(), QColor(theme::BG_DARK));

    if (m_result.cornerW <= 0 || m_result.cornerL <= 0 || m_result.cornerGrid.empty()) {
        painter.setPen(QColor(theme::TEXT_SECONDARY));
        painter.drawText(rect(), Qt::AlignCenter, "Enter pillar dimensions to view heightmap");
        return;
    }

    double cellW = static_cast<double>(width()) / m_result.cornerW;
    double cellH = static_cast<double>(height()) / m_result.cornerL;

    for (int y = 0; y < m_result.cornerL; ++y) {
        for (int x = 0; x < m_result.cornerW; ++x) {
            int h = m_result.cornerGrid[y][x];

            auto style = BridgePillarCalculator::getElevationStyle(h);
            QColor fillColor(style.fill.r, style.fill.g, style.fill.b);
            QColor borderColor(style.border.r, style.border.g, style.border.b);
            QColor textColor(style.text.r, style.text.g, style.text.b);

            QRectF cellRect(x * cellW, y * cellH, cellW, cellH);
            painter.fillRect(cellRect, fillColor);
            painter.setPen(QPen(borderColor, 1));
            painter.drawRect(cellRect);

            // Constrain text rendering within cell bounds with padding
            const qreal padX = 2.0;
            const qreal padY = 1.0;
            QRectF textRect = cellRect.adjusted(padX, padY, -padX, -padY);

            if (textRect.width() >= 12.0 && textRect.height() >= 10.0) {
                QString text = QString::number(h);
                QFont f = painter.font();
                int pixelSize = theme::FONT_SIZE_GRID_PX;
                f.setPixelSize(pixelSize);
                QFontMetrics fm(f);

                // Dynamically scale down font if needed to guarantee no overflow
                while (pixelSize > 7 && (fm.horizontalAdvance(text) > textRect.width() || fm.height() > textRect.height())) {
                    --pixelSize;
                    f.setPixelSize(pixelSize);
                    fm = QFontMetrics(f);
                }

                // Strictly enforce that text fits within textRect bounds without overlapping borders
                if (fm.horizontalAdvance(text) <= textRect.width() && fm.height() <= textRect.height()) {
                    painter.setFont(f);
                    painter.setPen(textColor);
                    painter.drawText(textRect, Qt::AlignCenter, text);
                }
            }
        }
    }

    // Highlight hovered cell
    if (m_hoverX >= 0 && m_hoverY >= 0 && m_hoverX < m_result.cornerW && m_hoverY < m_result.cornerL) {
        QRectF hoverRect(m_hoverX * cellW, m_hoverY * cellH, cellW, cellH);
        painter.setPen(QPen(QColor(theme::ACCENT_MINT), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(hoverRect.adjusted(1, 1, -1, -1));
    }
}

// --- BridgePillarWidget Implementation ---

BridgePillarWidget::BridgePillarWidget(QWidget* parent)
    : QWidget(parent) {
    setupUi();
    recalculate();
}

void BridgePillarWidget::setupUi() {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(14, 14, 14, 14);
    mainLayout->setSpacing(16);

    // Left controls & metrics column
    auto* leftCol = new QWidget(this);
    leftCol->setFixedWidth(340);
    auto* leftLayout = new QVBoxLayout(leftCol);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(12);

    auto* title = new QLabel("Bridge Dirt Pillar Calculator", leftCol);
    title->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(theme::ACCENT_MINT));
    leftLayout->addWidget(title);

    // Parameters Box
    auto* paramGroup = new QGroupBox("Plateau Parameters", leftCol);
    auto* paramForm = new QFormLayout(paramGroup);
    paramForm->setContentsMargins(12, 14, 12, 12);
    paramForm->setSpacing(8);

    m_topWSpin = new QSpinBox(paramGroup);
    m_topWSpin->setRange(1, 40);
    m_topWSpin->setValue(1);
    connect(m_topWSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &BridgePillarWidget::recalculate);
    paramForm->addRow("Top Width:", m_topWSpin);

    m_topLSpin = new QSpinBox(paramGroup);
    m_topLSpin->setRange(1, 40);
    m_topLSpin->setValue(2);
    connect(m_topLSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &BridgePillarWidget::recalculate);
    paramForm->addRow("Top Length:", m_topLSpin);

    m_heightSpin = new QSpinBox(paramGroup);
    m_heightSpin->setRange(1, 10000);
    m_heightSpin->setValue(1800);
    m_heightSpin->setSingleStep(100);
    m_heightSpin->setSuffix(" dirt");
    connect(m_heightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &BridgePillarWidget::recalculate);
    paramForm->addRow("Target Height:", m_heightSpin);

    m_skillCheck = new QCheckBox("Limit by Digging Skill", paramGroup);
    m_skillCheck->setObjectName("skillCheckBox");
    connect(m_skillCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_skillSpin->setEnabled(checked);
        recalculate();
    });
    paramForm->addRow(m_skillCheck);

    m_skillSpin = new QDoubleSpinBox(paramGroup);
    m_skillSpin->setObjectName("skillSpinBox");
    m_skillSpin->setRange(1.0, 100.0);
    m_skillSpin->setValue(50.0);
    m_skillSpin->setEnabled(false);
    connect(m_skillSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &BridgePillarWidget::recalculate);
    paramForm->addRow("Dig Skill:", m_skillSpin);

    m_pvpCheck = new QCheckBox("PvP Server Rules", paramGroup);
    m_pvpCheck->setObjectName("pvpCheckBox");
    m_pvpCheck->setToolTip("Halves maximum dirt slope (capped at 150) in accordance with PvP rules");
    connect(m_pvpCheck, &QCheckBox::toggled, this, &BridgePillarWidget::recalculate);
    paramForm->addRow(m_pvpCheck);

    leftLayout->addWidget(paramGroup);

    // Results Summary Box
    auto* resGroup = new QGroupBox("Dirt && Footprint Totals", leftCol);
    auto* resLayout = new QVBoxLayout(resGroup);
    resLayout->setContentsMargins(12, 14, 12, 12);
    resLayout->setSpacing(8);

    m_totalDirtLabel = new QLabel(resGroup);
    m_totalDirtLabel->setObjectName("totalDirtLabel");
    m_totalDirtLabel->setStyleSheet(QString("font-size: 20px; font-weight: bold; color: %1;").arg(theme::ACCENT_MINT));
    resLayout->addWidget(new QLabel("Required Dirt (Corner-Raises):", resGroup));
    resLayout->addWidget(m_totalDirtLabel);

    m_cratesLabel = new QLabel(resGroup);
    m_cratesLabel->setObjectName("cratesLabel");
    m_cratesLabel->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(theme::ACCENT_EMERALD));
    resLayout->addWidget(new QLabel("Full Dirt Crates (300 dirt/ea):", resGroup));
    resLayout->addWidget(m_cratesLabel);

    m_radiusLabel = new QLabel(resGroup);
    m_radiusLabel->setObjectName("radiusLabel");
    m_radiusLabel->setStyleSheet(QString("color: %1; font-weight: 500;").arg(theme::TEXT_PRIMARY));
    resLayout->addWidget(m_radiusLabel);

    m_footprintLabel = new QLabel(resGroup);
    m_footprintLabel->setObjectName("footprintLabel");
    m_footprintLabel->setStyleSheet(QString("color: %1; font-weight: 500;").arg(theme::TEXT_PRIMARY));
    resLayout->addWidget(m_footprintLabel);

    leftLayout->addWidget(resGroup);

    // Actions
    auto* copyBtn = new QPushButton("📋 Copy Heightmap Matrix", leftCol);
    copyBtn->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 8px; font-weight: 600;").arg(theme::SURFACE_CARD, theme::TEXT_PRIMARY, theme::BORDER_MUTED));
    connect(copyBtn, &QPushButton::clicked, this, &BridgePillarWidget::copyHeightmap);
    leftLayout->addWidget(copyBtn);

    leftLayout->addStretch(1);
    mainLayout->addWidget(leftCol);

    // Right visual elevation map column
    auto* rightCol = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightCol);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(8);

    auto* canvasHeader = new QHBoxLayout();
    auto* mapTitle = new QLabel("Interactive 2D Elevation Map (Corners)", rightCol);
    mapTitle->setStyleSheet(QString("font-weight: bold; color: %1;").arg(theme::ACCENT_MINT));
    m_hoverDetailLabel = new QLabel("Hover over a corner for details", rightCol);
    m_hoverDetailLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(theme::TEXT_SECONDARY));
    canvasHeader->addWidget(mapTitle);
    canvasHeader->addStretch(1);
    canvasHeader->addWidget(m_hoverDetailLabel);
    rightLayout->addLayout(canvasHeader);

    m_canvas = new PillarElevationCanvas();
    connect(m_canvas, &PillarElevationCanvas::cornerHovered, this, [this](int x, int y, int h) {
        if (x < 0 || y < 0) {
            m_hoverDetailLabel->setText("Hover over a corner for details");
        } else {
            int tier = BridgePillarCalculator::getElevationTier(h);
            m_hoverDetailLabel->setText(QString("Corner [%1, %2] | Height: %3 dirt (Tier %4)").arg(x).arg(y).arg(h).arg(tier));
        }
    });

    m_scrollArea = new QScrollArea(rightCol);
    m_scrollArea->setWidget(m_canvas);
    m_scrollArea->setWidgetResizable(false);
    m_scrollArea->viewport()->installEventFilter(this);
    m_scrollArea->setStyleSheet(QString(
        "QScrollArea { background-color: %1; border: 1px solid %2; border-radius: 6px; }"
        "QScrollBar:vertical, QScrollBar:horizontal { background: %3; border: none; }"
        "QScrollBar::handle:vertical, QScrollBar::handle:horizontal { background: %4; border-radius: 4px; min-height: 20px; min-width: 20px; }"
        "QScrollBar::handle:hover { background: %5; }"
    ).arg(theme::BG_DARK, theme::BORDER_MUTED, theme::SURFACE_DARK, theme::SURFACE_ACTIVE, theme::ACCENT_EMERALD));

    rightLayout->addWidget(m_scrollArea, 1);

    mainLayout->addWidget(rightCol, 1);
}

bool BridgePillarWidget::eventFilter(QObject* obj, QEvent* event) {
    if (m_scrollArea && obj == m_scrollArea->viewport() && event->type() == QEvent::Resize) {
        updateCanvasSize();
    }
    return QWidget::eventFilter(obj, event);
}

void BridgePillarWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    updateCanvasSize();
}

void BridgePillarWidget::updateCanvasSize() {
    if (!m_scrollArea || !m_canvas) return;
    QSize vpSize = m_scrollArea->viewport()->size();
    if (vpSize.width() <= 0 || vpSize.height() <= 0) {
        vpSize = m_scrollArea->size();
    }
    m_canvas->updateGeometryForViewport(vpSize);
}

void BridgePillarWidget::recalculate() {
    int topW = m_topWSpin->value();
    int topL = m_topLSpin->value();
    int height = m_heightSpin->value();
    std::optional<double> digSkill = std::nullopt;
    if (m_skillCheck->isChecked()) {
        digSkill = m_skillSpin->value();
    }

    bool isPvp = m_pvpCheck && m_pvpCheck->isChecked();
    m_lastResult = BridgePillarCalculator::calculate(topW, topL, height, digSkill, isPvp);

    QLocale locale;
    m_totalDirtLabel->setText(locale.toString(m_lastResult.totalDirt) + " dirt");
    m_cratesLabel->setText(locale.toString(m_lastResult.crates) + " crates");
    m_radiusLabel->setText(QString("Plateau: %1x%2 | Radius: %3 tiles (Slope: %4%5)")
        .arg(m_lastResult.plateauCornersX).arg(m_lastResult.plateauCornersY)
        .arg(m_lastResult.spreadRadius).arg(m_lastResult.effectiveSlope)
        .arg(m_lastResult.isPvp ? " [PvP]" : ""));
    m_footprintLabel->setText(QString("Base Footprint: %1x%2 corners")
        .arg(m_lastResult.cornerW).arg(m_lastResult.cornerL));

    m_canvas->setResult(m_lastResult);
    updateCanvasSize();
}

void BridgePillarWidget::copyHeightmap() {
    QString out;
    out += QString("Wurm Dirt Pillar (%1x%2 Top, %3 Height, Base: %4x%5 Corners)\n")
        .arg(m_lastResult.topW).arg(m_lastResult.topL)
        .arg(m_lastResult.targetHeight)
        .arg(m_lastResult.cornerW).arg(m_lastResult.cornerL);
    out += QString("Total Dirt: %1 (%2 Crates)\n\n").arg(m_lastResult.totalDirt).arg(m_lastResult.crates);

    for (int y = 0; y < m_lastResult.cornerL; ++y) {
        for (int x = 0; x < m_lastResult.cornerW; ++x) {
            out += QString("%1\t").arg(m_lastResult.cornerGrid[y][x]);
        }
        out += "\n";
    }

    QApplication::clipboard()->setText(out);
}

} // namespace tools
