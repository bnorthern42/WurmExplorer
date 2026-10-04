#include "GrinderChartWidget.hpp"
#include "../../ui/ThemeTokens.hpp"

#include <QPainter>
#include <QMouseEvent>
#include <cmath>
#include <algorithm>

namespace tools {

using namespace treasure::ui;

GrinderChartWidget::GrinderChartWidget(QWidget* parent)
    : QWidget(parent) {
    setMouseTracking(true);
    setMinimumHeight(240);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void GrinderChartWidget::setDistribution(const RollDistribution& dist, const QString& title) {
    m_dist = dist;
    m_chartTitle = title;
    m_hoverBucket = -9999;
    update();
}

void GrinderChartWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_dist.histogram.empty()) return;

    int minB = m_dist.histogram.begin()->first;
    int maxB = m_dist.histogram.rbegin()->first;
    int span = std::max(1, maxB - minB + 1);

    double plotLeft = 45.0;
    double plotRight = width() - 20.0;
    double plotWidth = plotRight - plotLeft;

    double mx = event->position().x();
    if (mx >= plotLeft && mx <= plotRight) {
        double ratio = (mx - plotLeft) / plotWidth;
        int bucket = minB + static_cast<int>(std::round(ratio * (span - 1)));
        if (bucket != m_hoverBucket) {
            m_hoverBucket = bucket;
            update();
        }
    } else {
        leaveEvent(nullptr);
    }
}

void GrinderChartWidget::leaveEvent(QEvent*) {
    if (m_hoverBucket != -9999) {
        m_hoverBucket = -9999;
        update();
    }
}

void GrinderChartWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.fillRect(rect(), QColor(theme::SURFACE_DARK));

    // Frame border
    painter.setPen(QColor(theme::BORDER_MUTED));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));

    if (m_dist.histogram.empty()) {
        painter.setPen(QColor(theme::TEXT_SECONDARY));
        painter.drawText(rect(), Qt::AlignCenter, "Run simulation to view probability distribution");
        return;
    }

    double plotLeft = 45.0;
    double plotRight = width() - 20.0;
    double plotTop = 30.0;
    double plotBottom = height() - 30.0;
    double plotWidth = plotRight - plotLeft;
    double plotHeight = plotBottom - plotTop;

    int minB = m_dist.histogram.begin()->first;
    int maxB = m_dist.histogram.rbegin()->first;
    int span = std::max(1, maxB - minB + 1);

    int maxCount = 0;
    for (const auto& [b, c] : m_dist.histogram) {
        if (c > maxCount) maxCount = c;
    }
    if (maxCount == 0) maxCount = 1;

    // Title
    painter.setPen(QColor(theme::ACCENT_MINT));
    painter.setFont(QFont("sans-serif", 10, QFont::Bold));
    painter.drawText(QPointF(plotLeft, 20), m_chartTitle);

    // Draw gridlines
    painter.setPen(QPen(QColor(theme::BORDER_MUTED), 1, Qt::DotLine));
    painter.drawLine(QPointF(plotLeft, plotTop), QPointF(plotRight, plotTop));
    painter.drawLine(QPointF(plotLeft, plotTop + plotHeight * 0.5), QPointF(plotRight, plotTop + plotHeight * 0.5));
    painter.drawLine(QPointF(plotLeft, plotBottom), QPointF(plotRight, plotBottom));

    // Draw bars
    double barWidth = std::max(2.0, (plotWidth / span) * 0.85);

    for (const auto& [b, c] : m_dist.histogram) {
        double x = plotLeft + (static_cast<double>(b - minB) / span) * plotWidth;
        double barH = (static_cast<double>(c) / maxCount) * plotHeight;
        double y = plotBottom - barH;

        QColor barColor(theme::ACCENT_EMERALD);
        if (b == m_hoverBucket) {
            barColor = QColor(theme::ACCENT_MINT);
        }

        painter.fillRect(QRectF(x, y, barWidth, barH), barColor);
    }

    // Axes and tick marks
    painter.setPen(QColor(theme::TEXT_SECONDARY));
    painter.setFont(QFont("sans-serif", 8));

    // Y-Axis counts
    painter.drawText(QRectF(0, plotTop - 8, 40, 16), Qt::AlignRight, QString::number(maxCount));
    painter.drawText(QRectF(0, plotTop + plotHeight * 0.5 - 8, 40, 16), Qt::AlignRight, QString::number(maxCount / 2));
    painter.drawText(QRectF(0, plotBottom - 8, 40, 16), Qt::AlignRight, "0");

    // X-Axis range labels
    painter.drawText(QRectF(plotLeft, plotBottom + 5, 50, 16), Qt::AlignLeft, QString::number(minB));
    painter.drawText(QRectF(plotRight - 50, plotBottom + 5, 50, 16), Qt::AlignRight, QString::number(maxB));

    // Hover tooltip
    if (m_hoverBucket != -9999) {
        int count = 0;
        auto it = m_dist.histogram.find(m_hoverBucket);
        if (it != m_dist.histogram.end()) {
            count = it->second;
        }
        double pct = (static_cast<double>(count) / m_dist.totalTrials) * 100.0;
        QString tip = QString("Val: %1 | Count: %2 (%3%)").arg(m_hoverBucket).arg(count).arg(pct, 0, 'f', 1);

        QFontMetrics fm(painter.font());
        int tw = fm.horizontalAdvance(tip) + 12;
        int th = fm.height() + 6;

        double hx = plotLeft + (static_cast<double>(m_hoverBucket - minB) / span) * plotWidth;
        hx = std::clamp(hx - tw / 2.0, plotLeft, plotRight - tw);
        double hy = plotTop + 10;

        painter.fillRect(QRectF(hx, hy, tw, th), QColor(theme::SURFACE_CARD));
        painter.setPen(QColor(theme::ACCENT_MINT));
        painter.drawRect(QRectF(hx, hy, tw, th));
        painter.drawText(QRectF(hx, hy, tw, th), Qt::AlignCenter, tip);
    }
}

} // namespace tools
