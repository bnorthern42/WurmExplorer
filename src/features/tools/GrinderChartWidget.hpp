#pragma once

#include <QWidget>
#include <QString>
#include "GrinderEngine.hpp"

namespace tools {

class GrinderChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit GrinderChartWidget(QWidget* parent = nullptr);
    void setDistribution(const RollDistribution& dist, const QString& title);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    RollDistribution m_dist;
    QString m_chartTitle;
    int m_hoverBucket = -9999;
};

} // namespace tools
