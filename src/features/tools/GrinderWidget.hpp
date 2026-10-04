#pragma once

#include <QWidget>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <vector>
#include "GrinderEngine.hpp"
#include "../skills/SkillTracker.hpp"

namespace tools {

class GrinderChartWidget;

class GrinderWidget : public QWidget {
    Q_OBJECT
public:
    explicit GrinderWidget(QWidget* parent = nullptr);

public slots:
    void onSkillsUpdated(const std::vector<skills::SkillStats>& stats);

private slots:
    void onActionChanged(int index);
    void onLinkSkillsToggled(bool checked);
    void onMobChanged(int index);
    void onDifficultyPresetChanged(int index);
    void onDifficultySpinChanged(double val);
    void runSimulation();

private:
    void setupUi();
    void updateModeVisibility();
    void populateDifficultyPresets(ActionMode mode);
    void syncSkillsFromStats();

    QComboBox* m_actionCombo = nullptr;
    QCheckBox* m_linkSkillsCheck = nullptr;
    QLabel* m_linkStatusLabel = nullptr;

    // Parameters form
    QFormLayout* m_form = nullptr;

    // Dynamic inputs
    QLabel* m_primarySkillLabel = nullptr;
    QDoubleSpinBox* m_primarySkillSpin = nullptr;

    QLabel* m_secondarySkillLabel = nullptr;
    QDoubleSpinBox* m_secondarySkillSpin = nullptr;

    QLabel* m_tertiarySkillLabel = nullptr;
    QDoubleSpinBox* m_tertiarySkillSpin = nullptr;

    QLabel* m_toolQlLabel = nullptr;
    QDoubleSpinBox* m_toolQlSpin = nullptr;

    QLabel* m_difficultyLabel = nullptr;
    QWidget* m_difficultyContainer = nullptr;
    QComboBox* m_difficultyCombo = nullptr;
    QDoubleSpinBox* m_difficultySpin = nullptr;

    // Mode-specific rows
    QLabel* m_mobLabel = nullptr;
    QComboBox* m_mobCombo = nullptr;

    QLabel* m_materialQlLabel = nullptr;
    QDoubleSpinBox* m_materialQlSpin = nullptr;

    QLabel* m_startQlLabel = nullptr;
    QDoubleSpinBox* m_startQlSpin = nullptr;

    QLabel* m_targetQlLabel = nullptr;
    QDoubleSpinBox* m_targetQlSpin = nullptr;

    QLabel* m_veinQlLabel = nullptr;
    QDoubleSpinBox* m_veinQlSpin = nullptr;

    QLabel* m_imbueLabel = nullptr;
    QSpinBox* m_imbueSpin = nullptr;

    QLabel* m_rarityLabel = nullptr;
    QDoubleSpinBox* m_raritySpin = nullptr;

    QLabel* m_runeLabel = nullptr;
    QDoubleSpinBox* m_runeSpin = nullptr;

    QLabel* m_slopeLabel = nullptr;
    QDoubleSpinBox* m_slopeSpin = nullptr;

    QLabel* m_pathLevelLabel = nullptr;
    QSpinBox* m_pathLevelSpin = nullptr;

    QLabel* m_sheepAgeLabel = nullptr;
    QSpinBox* m_sheepAgeSpin = nullptr;

    QCheckBox* m_benedictionCheck = nullptr;
    QCheckBox* m_mediTileCheck = nullptr;
    QCheckBox* m_mediCooldownCheck = nullptr;
    QCheckBox* m_isFoCheck = nullptr;
    QCheckBox* m_isHotsCheck = nullptr;
    QCheckBox* m_isTamedCheck = nullptr;

    // KPI Results
    QLabel* m_successRateLabel = nullptr;
    QLabel* m_meanLabel = nullptr;
    QLabel* m_rangeLabel = nullptr;
    GrinderChartWidget* m_chart = nullptr;

    // Cached skill stats from character log
    std::vector<skills::SkillStats> m_latestStats;
    bool m_initialized = false;
};

} // namespace tools
