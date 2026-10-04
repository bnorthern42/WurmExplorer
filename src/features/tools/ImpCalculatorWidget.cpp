#include "ImpCalculatorWidget.hpp"
#include "ImpCalculator.hpp"
#include "../../ui/ThemeTokens.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>

namespace tools {

using namespace treasure::ui;

ImpCalculatorWidget::ImpCalculatorWidget(QWidget* parent)
    : QWidget(parent) {
    setupUi();
    recalculate();
}

void ImpCalculatorWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // Title
    auto* header = new QLabel("Wurm Online Imp & Skill QL Calculator", this);
    header->setStyleSheet(QString("font-size: 18px; font-weight: bold; color: %1;").arg(theme::ACCENT_MINT));
    mainLayout->addWidget(header);

    auto* subheader = new QLabel("Calculate maximum impable quality for your skill level, or find required skill for desired QL.", this);
    subheader->setStyleSheet(QString("color: %1; font-size: 12px;").arg(theme::TEXT_SECONDARY));
    mainLayout->addWidget(subheader);

    // Inputs Group
    auto* inputGroup = new QGroupBox("Calculator Parameters", this);
    auto* inputGrid = new QGridLayout(inputGroup);
    inputGrid->setContentsMargins(14, 16, 14, 14);
    inputGrid->setSpacing(12);

    // 1: Current Skill
    auto* skillLabel = new QLabel("Current Skill:", inputGroup);
    skillLabel->setStyleSheet(QString("color: %1; font-weight: 600;").arg(theme::TEXT_PRIMARY));
    m_skillSpin = new QDoubleSpinBox(inputGroup);
    m_skillSpin->setRange(0.0, 100.0);
    m_skillSpin->setValue(50.0);
    m_skillSpin->setDecimals(2);
    m_skillSpin->setSingleStep(1.0);
    connect(m_skillSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ImpCalculatorWidget::recalculate);

    // 2: Desired QL
    auto* qlLabel = new QLabel("Target QL Wanted:", inputGroup);
    qlLabel->setStyleSheet(QString("color: %1; font-weight: 600;").arg(theme::TEXT_PRIMARY));
    m_targetQlSpin = new QDoubleSpinBox(inputGroup);
    m_targetQlSpin->setRange(0.0, 100.0);
    m_targetQlSpin->setValue(70.0);
    m_targetQlSpin->setDecimals(2);
    m_targetQlSpin->setSingleStep(1.0);
    connect(m_targetQlSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ImpCalculatorWidget::recalculate);

    // 3: Imbue
    auto* imbueLabel = new QLabel("Tool Imbue Bonus (Optional):", inputGroup);
    imbueLabel->setStyleSheet(QString("color: %1; font-weight: 600;").arg(theme::TEXT_PRIMARY));
    m_imbueSpin = new QSpinBox(inputGroup);
    m_imbueSpin->setRange(0, 999);
    m_imbueSpin->setValue(0);
    connect(m_imbueSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ImpCalculatorWidget::recalculate);

    inputGrid->addWidget(skillLabel, 0, 0);
    inputGrid->addWidget(m_skillSpin, 0, 1);
    inputGrid->addWidget(qlLabel, 1, 0);
    inputGrid->addWidget(m_targetQlSpin, 1, 1);
    inputGrid->addWidget(imbueLabel, 2, 0);
    inputGrid->addWidget(m_imbueSpin, 2, 1);

    mainLayout->addWidget(inputGroup);

    // Results Display Cards (Two big cards side by side)
    auto* cardsRow = new QHBoxLayout();
    cardsRow->setSpacing(14);

    // Card 1: Max Imp QL
    auto* card1 = new QFrame(this);
    card1->setStyleSheet(QString(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 14px; }"
    ).arg(theme::SURFACE_CARD, theme::BORDER_MUTED));
    auto* card1Layout = new QVBoxLayout(card1);
    card1Layout->setSpacing(6);
    auto* card1Title = new QLabel("MAXIMUM IMP QUALITY", card1);
    card1Title->setStyleSheet(QString("font-size: 11px; font-weight: bold; color: %1;").arg(theme::TEXT_SECONDARY));
    m_maxQlOutput = new QLabel("0.00", card1);
    m_maxQlOutput->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;").arg(theme::ACCENT_MINT));
    auto* card1Desc = new QLabel("Maximum QL you can reach at this skill level", card1);
    card1Desc->setStyleSheet(QString("font-size: 11px; color: %1;").arg(theme::TEXT_SECONDARY));
    card1Layout->addWidget(card1Title);
    card1Layout->addWidget(m_maxQlOutput);
    card1Layout->addWidget(card1Desc);
    cardsRow->addWidget(card1);

    // Card 2: Skill Needed
    auto* card2 = new QFrame(this);
    card2->setStyleSheet(QString(
        "QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 14px; }"
    ).arg(theme::SURFACE_CARD, theme::BORDER_MUTED));
    auto* card2Layout = new QVBoxLayout(card2);
    card2Layout->setSpacing(6);
    auto* card2Title = new QLabel("REQUIRED SKILL NEEDED", card2);
    card2Title->setStyleSheet(QString("font-size: 11px; font-weight: bold; color: %1;").arg(theme::TEXT_SECONDARY));
    m_skillNeededOutput = new QLabel("0.00", card2);
    m_skillNeededOutput->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;").arg(theme::ACCENT_EMERALD));
    auto* card2Desc = new QLabel("Skill level required to reach desired target QL", card2);
    card2Desc->setStyleSheet(QString("font-size: 11px; color: %1;").arg(theme::TEXT_SECONDARY));
    card2Layout->addWidget(card2Title);
    card2Layout->addWidget(m_skillNeededOutput);
    card2Layout->addWidget(card2Desc);
    cardsRow->addWidget(card2);

    mainLayout->addLayout(cardsRow);

    // Quick Reference Table
    auto* refGroup = new QGroupBox("Standard Skill Reference Milestones", this);
    auto* refLayout = new QVBoxLayout(refGroup);

    m_refTable = new QTableWidget(this);
    m_refTable->setColumnCount(4);
    m_refTable->setHorizontalHeaderLabels({"Skill Level", "Max Imp QL", "Target QL", "Skill Required"});
    m_refTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_refTable->verticalHeader()->setVisible(false);
    m_refTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_refTable->setAlternatingRowColors(true);
    m_refTable->setMaximumHeight(220);

    const std::vector<double> milestones = {20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 99.0};
    m_refTable->setRowCount(static_cast<int>(milestones.size()));

    for (int i = 0; i < static_cast<int>(milestones.size()); ++i) {
        double sk = milestones[i];
        double maxQ = ImpCalculator::calculateMaxImpQl(sk, 0);
        double needed = ImpCalculator::calculateSkillNeeded(sk, 0);

        m_refTable->setItem(i, 0, new QTableWidgetItem(QString::number(sk, 'f', 1)));
        auto* maxItem = new QTableWidgetItem(QString::number(maxQ, 'f', 2));
        maxItem->setForeground(QColor(theme::ACCENT_MINT));
        m_refTable->setItem(i, 1, maxItem);

        m_refTable->setItem(i, 2, new QTableWidgetItem(QString::number(sk, 'f', 1)));
        auto* needItem = new QTableWidgetItem(needed > 0.0 ? QString::number(needed, 'f', 2) : "< 1");
        needItem->setForeground(QColor(theme::ACCENT_EMERALD));
        m_refTable->setItem(i, 3, needItem);
    }

    refLayout->addWidget(m_refTable);
    mainLayout->addWidget(refGroup);
    mainLayout->addStretch(1);
}

void ImpCalculatorWidget::recalculate() {
    double skill = m_skillSpin->value();
    double targetQl = m_targetQlSpin->value();
    int imbue = m_imbueSpin->value();

    double maxQl = ImpCalculator::calculateMaxImpQl(skill, imbue);
    double skillNeeded = ImpCalculator::calculateSkillNeeded(targetQl, imbue);

    m_maxQlOutput->setText(QString::number(maxQl, 'f', 2));
    if (targetQl < 23.77) {
        m_skillNeededOutput->setText("< 1.0 (Any)");
    } else {
        m_skillNeededOutput->setText(QString::number(skillNeeded, 'f', 2));
    }
}

} // namespace tools
