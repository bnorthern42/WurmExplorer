#pragma once

#include <QWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QTableWidget>

namespace tools {

class ImpCalculatorWidget : public QWidget {
    Q_OBJECT
public:
    explicit ImpCalculatorWidget(QWidget* parent = nullptr);

private slots:
    void recalculate();

private:
    void setupUi();

    QDoubleSpinBox* m_skillSpin = nullptr;
    QDoubleSpinBox* m_targetQlSpin = nullptr;
    QSpinBox* m_imbueSpin = nullptr;

    QLabel* m_maxQlOutput = nullptr;
    QLabel* m_skillNeededOutput = nullptr;
    QTableWidget* m_refTable = nullptr;
};

} // namespace tools
