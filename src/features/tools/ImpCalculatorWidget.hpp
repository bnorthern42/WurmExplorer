#pragma once

#include <QWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QTableWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>

namespace tools {

class ImpCalculatorWidget : public QWidget {
    Q_OBJECT
public:
    explicit ImpCalculatorWidget(QWidget* parent = nullptr);

    bool isLiveSyncEnabled() const;
    double currentSkill() const;
    double maxImpQl() const;
    void setCurrentSkill(double skill);

private slots:
    void recalculate();
    void onSkillUpdated(const QString& name, double level);
    void onLiveSyncToggled(bool checked);
    void onSkillSelectionChanged(const QString& skillName);

private:
    void setupUi();

    QCheckBox* m_liveSyncCheck = nullptr;
    QComboBox* m_skillCombo = nullptr;
    QDoubleSpinBox* m_skillSpin = nullptr;
    QDoubleSpinBox* m_targetQlSpin = nullptr;
    QSpinBox* m_imbueSpin = nullptr;

    QLabel* m_maxQlOutput = nullptr;
    QLabel* m_skillNeededOutput = nullptr;
    QTableWidget* m_refTable = nullptr;
};

} // namespace tools
