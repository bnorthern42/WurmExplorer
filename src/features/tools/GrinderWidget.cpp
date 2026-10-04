#include "GrinderWidget.hpp"
#include "GrinderChartWidget.hpp"
#include "GrinderMobData.hpp"
#include "GrinderModeConfig.hpp"
#include "DifficultyPresets.hpp"
#include "../../ui/ThemeTokens.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <algorithm>

namespace tools {

using namespace treasure::ui;

GrinderWidget::GrinderWidget(QWidget* parent)
    : QWidget(parent) {
    setupUi();
    m_initialized = true;
    populateDifficultyPresets(static_cast<ActionMode>(m_actionCombo->currentIndex()));
    updateModeVisibility();
}

void GrinderWidget::setupUi() {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    auto* leftScroll = new QScrollArea(this);
    leftScroll->setWidgetResizable(true);
    leftScroll->setMinimumWidth(360);
    leftScroll->setMaximumWidth(400);
    leftScroll->setStyleSheet(QString("QScrollArea { border: none; background: transparent; }"));

    auto* leftCol = new QWidget(leftScroll);
    auto* leftLayout = new QVBoxLayout(leftCol);
    leftLayout->setContentsMargins(0, 0, 8, 0);
    leftLayout->setSpacing(8);

    // 1. Action Mode Selector
    auto* modeBox = new QGroupBox("Action Mode", leftCol);
    auto* mbLayout = new QVBoxLayout(modeBox);
    m_actionCombo = new QComboBox(modeBox);
    m_actionCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_actionCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_actionCombo->setMinimumContentsLength(10);
    m_actionCombo->addItems({
        "Fixed / Generic Skill Check",
        "Mining (Action Power)",
        "Mining (Ore Quality)",
        "Farming (Rake & Nature)",
        "Digging (Slope & Shovel)",
        "Meditation (Tile & Path)",
        "Creation (Tools & Materials)",
        "Woodcutting (Log / Timber QL)",
        "Imping (Target Quality)",
        "Smithing (Multi-Step Improvement)",
        "Taming (Creature CR & Soul)",
        "Fileting (Fish Butchery & Cooking)",
        "Forestry (Sprout Harvesting)",
        "Shearing (Sheep Age & Scissors)"
    });
    m_actionCombo->setStyleSheet(QString("QComboBox { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 6px; font-weight: bold; }")
        .arg(theme::SURFACE_CARD, theme::TEXT_PRIMARY, theme::BORDER_MUTED));
    connect(m_actionCombo, &QComboBox::currentIndexChanged, this, &GrinderWidget::onActionChanged);
    mbLayout->addWidget(m_actionCombo);

    m_linkSkillsCheck = new QCheckBox("🔗 Link with Character Skills (Live Sync)", modeBox);
    m_linkSkillsCheck->setStyleSheet(QString("QCheckBox { color: %1; font-weight: 600; margin-top: 4px; } QCheckBox::indicator:checked { background-color: %2; border: 1px solid %3; }")
        .arg(theme::ACCENT_MINT, theme::ACCENT_EMERALD, theme::ACCENT_MINT));
    connect(m_linkSkillsCheck, &QCheckBox::toggled, this, &GrinderWidget::onLinkSkillsToggled);
    mbLayout->addWidget(m_linkSkillsCheck);

    m_linkStatusLabel = new QLabel("Live character sync inactive", modeBox);
    m_linkStatusLabel->setWordWrap(true);
    m_linkStatusLabel->setStyleSheet(QString("color: %1; font-size: 11px; padding: 2px;").arg(theme::TEXT_SECONDARY));
    mbLayout->addWidget(m_linkStatusLabel);

    leftLayout->addWidget(modeBox);

    // 2. Parameters GroupBox
    auto* paramBox = new QGroupBox("Parameters", leftCol);
    m_form = new QFormLayout(paramBox);
    m_form->setSpacing(6);
    m_form->setContentsMargins(8, 10, 8, 8);
    m_form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    m_form->setLabelAlignment(Qt::AlignLeft);

    auto createDblSpin = [this](double val, double minV, double maxV, double step = 1.0, int dec = 2) {
        auto* sb = new QDoubleSpinBox(this);
        sb->setRange(minV, maxV);
        sb->setValue(val);
        sb->setSingleStep(step);
        sb->setDecimals(dec);
        connect(sb, &QDoubleSpinBox::valueChanged, this, &GrinderWidget::runSimulation);
        return sb;
    };

    auto createIntSpin = [this](int val, int minV, int maxV) {
        auto* sb = new QSpinBox(this);
        sb->setRange(minV, maxV);
        sb->setValue(val);
        connect(sb, &QSpinBox::valueChanged, this, &GrinderWidget::runSimulation);
        return sb;
    };

    auto addDbl = [&](QLabel*& lbl, const QString& title, QDoubleSpinBox*& spin, double val, double minV, double maxV, double step = 1.0, int dec = 2) {
        lbl = new QLabel(title, paramBox);
        spin = createDblSpin(val, minV, maxV, step, dec);
        m_form->addRow(lbl, spin);
    };

    auto addInt = [&](QLabel*& lbl, const QString& title, QSpinBox*& spin, int val, int minV, int maxV) {
        lbl = new QLabel(title, paramBox);
        spin = createIntSpin(val, minV, maxV);
        m_form->addRow(lbl, spin);
    };

    addDbl(m_primarySkillLabel, "Primary Skill:", m_primarySkillSpin, 50.0, 1.0, 100.0, 0.5, 2);
    addDbl(m_secondarySkillLabel, "Secondary Skill:", m_secondarySkillSpin, 50.0, 1.0, 100.0, 0.5, 2);
    addDbl(m_tertiarySkillLabel, "Tertiary Skill:", m_tertiarySkillSpin, 40.0, 1.0, 100.0, 0.5, 2);
    addDbl(m_toolQlLabel, "Tool / Item QL:", m_toolQlSpin, 50.0, 1.0, 100.0, 1.0, 2);

    m_difficultyLabel = new QLabel("Difficulty:", paramBox);

    m_difficultyContainer = new QWidget(paramBox);
    m_difficultyContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto* diffLayout = new QHBoxLayout(m_difficultyContainer);
    diffLayout->setContentsMargins(0, 0, 0, 0);
    diffLayout->setSpacing(6);

    m_difficultyCombo = new QComboBox(m_difficultyContainer);
    m_difficultyCombo->setStyleSheet(QString("QComboBox { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 4px 6px; }")
        .arg(theme::SURFACE_CARD, theme::TEXT_PRIMARY, theme::BORDER_MUTED));
    m_difficultyCombo->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_difficultyCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_difficultyCombo->setMinimumContentsLength(8);

    m_difficultySpin = new QDoubleSpinBox(m_difficultyContainer);
    m_difficultySpin->setStyleSheet(QString("QDoubleSpinBox { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 4px; }")
        .arg(theme::SURFACE_CARD, theme::TEXT_PRIMARY, theme::BORDER_MUTED));
    m_difficultySpin->setRange(0.0, 10000.0);
    m_difficultySpin->setValue(20.0);
    m_difficultySpin->setSingleStep(1.0);
    m_difficultySpin->setDecimals(2);
    m_difficultySpin->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_difficultySpin->setMinimumWidth(75);

    diffLayout->addWidget(m_difficultyCombo, 2);
    diffLayout->addWidget(m_difficultySpin, 1);
    m_form->addRow(m_difficultyLabel, m_difficultyContainer);

    connect(m_difficultyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GrinderWidget::onDifficultyPresetChanged);
    connect(m_difficultySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &GrinderWidget::onDifficultySpinChanged);

    m_mobLabel = new QLabel("Target Creature:", paramBox);
    m_mobCombo = new QComboBox(paramBox);
    m_mobCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_mobCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_mobCombo->setMinimumContentsLength(10);
    const auto& mobs = getTamingMobs();
    std::vector<std::string> mobNames;
    for (const auto& [name, _] : mobs) mobNames.push_back(name);
    std::sort(mobNames.begin(), mobNames.end());
    for (const auto& name : mobNames) {
        const auto& d = mobs.at(name);
        m_mobCombo->addItem(QString("%1 (sstr %2, cr %3)").arg(QString::fromStdString(name)).arg(d.sstr, 0, 'f', 0).arg(d.cr, 0, 'f', 0), QString::fromStdString(name));
    }
    connect(m_mobCombo, &QComboBox::currentIndexChanged, this, &GrinderWidget::onMobChanged);
    m_form->addRow(m_mobLabel, m_mobCombo);

    addDbl(m_materialQlLabel, "Material QL:", m_materialQlSpin, 50.0, 1.0, 100.0);
    addDbl(m_startQlLabel, "Start QL:", m_startQlSpin, 30.0, 1.0, 99.0);
    addDbl(m_targetQlLabel, "Target QL:", m_targetQlSpin, 70.0, 1.0, 100.0);
    addDbl(m_veinQlLabel, "Vein QL:", m_veinQlSpin, 50.0, 1.0, 100.0);
    addInt(m_imbueLabel, "Tool Imbue:", m_imbueSpin, 0, 0, 100);
    addDbl(m_rarityLabel, "Tool Rarity:", m_raritySpin, 0.0, 0.0, 3.0, 1.0, 0);
    addDbl(m_runeLabel, "Rune Bonus:", m_runeSpin, 0.0, 0.0, 1.0, 0.05, 2);
    addDbl(m_slopeLabel, "Dig Slope:", m_slopeSpin, 0.0, 0.0, 300.0, 5.0, 1);
    addInt(m_pathLevelLabel, "Path Level:", m_pathLevelSpin, 5, 1, 15);
    addInt(m_sheepAgeLabel, "Sheep Age:", m_sheepAgeSpin, 15, 1, 50);

    auto makeCheck = [this](const QString& text) {
        auto* cb = new QCheckBox(text, this);
        connect(cb, &QCheckBox::toggled, this, &GrinderWidget::runSimulation);
        return cb;
    };
    m_benedictionCheck = makeCheck("Benediction (+5)");
    m_mediTileCheck = makeCheck("On Special Tile"); m_mediTileCheck->setChecked(true);
    m_mediCooldownCheck = makeCheck("Cooldown Active");
    m_isFoCheck = makeCheck("Fo Bonus (+20)");
    m_isHotsCheck = makeCheck("Hots Server");
    m_isTamedCheck = makeCheck("Already Tamed");

    m_form->addRow(m_benedictionCheck);
    m_form->addRow(m_mediTileCheck);
    m_form->addRow(m_mediCooldownCheck);
    m_form->addRow(m_isFoCheck);
    m_form->addRow(m_isHotsCheck);
    m_form->addRow(m_isTamedCheck);

    leftLayout->addWidget(paramBox);

    // Results KPI Group
    auto* resGroup = new QGroupBox("Simulation Statistics", leftCol);
    auto* resLayout = new QVBoxLayout(resGroup);
    m_successRateLabel = new QLabel(resGroup);
    m_meanLabel = new QLabel(resGroup);
    m_rangeLabel = new QLabel(resGroup);
    resLayout->addWidget(m_successRateLabel);
    resLayout->addWidget(m_meanLabel);
    resLayout->addWidget(m_rangeLabel);
    leftLayout->addWidget(resGroup);

    leftLayout->addStretch(1);
    leftScroll->setWidget(leftCol);
    mainLayout->addWidget(leftScroll);

    // Right Column: Distribution Chart View
    auto* rightCol = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightCol);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    m_chart = new GrinderChartWidget(rightCol);
    rightLayout->addWidget(m_chart);
    mainLayout->addWidget(rightCol, 1);
}

void GrinderWidget::updateModeVisibility() {
    auto mode = static_cast<ActionMode>(m_actionCombo->currentIndex());
    auto cfg = getModeConfig(mode);

    m_primarySkillLabel->setText(cfg.primarySkillLabel);
    m_secondarySkillLabel->setText(cfg.secondarySkillLabel);
    m_tertiarySkillLabel->setText(cfg.tertiarySkillLabel);
    m_toolQlLabel->setText(cfg.toolQlLabel);
    m_difficultyLabel->setText(cfg.difficultyLabel);

    auto setRowVis = [](QLabel* lbl, QWidget* w, bool vis) {
        if (lbl) lbl->setVisible(vis);
        if (w) w->setVisible(vis);
    };

    setRowVis(m_secondarySkillLabel, m_secondarySkillSpin, cfg.showSecondarySkill);
    setRowVis(m_tertiarySkillLabel, m_tertiarySkillSpin, cfg.showTertiarySkill);
    setRowVis(m_toolQlLabel, m_toolQlSpin, cfg.showToolQl);
    setRowVis(m_difficultyLabel, m_difficultyContainer, cfg.showDifficulty);
    setRowVis(m_mobLabel, m_mobCombo, cfg.showMob);
    setRowVis(m_materialQlLabel, m_materialQlSpin, cfg.showMaterialQl);
    setRowVis(m_startQlLabel, m_startQlSpin, cfg.showStartQl);
    setRowVis(m_targetQlLabel, m_targetQlSpin, cfg.showTargetQl);
    setRowVis(m_veinQlLabel, m_veinQlSpin, cfg.showVeinQl);
    setRowVis(m_imbueLabel, m_imbueSpin, cfg.showImbue);
    setRowVis(m_rarityLabel, m_raritySpin, cfg.showRarity);
    setRowVis(m_runeLabel, m_runeSpin, cfg.showRune);
    setRowVis(m_slopeLabel, m_slopeSpin, cfg.showSlope);
    setRowVis(m_pathLevelLabel, m_pathLevelSpin, cfg.showPathLevel);
    setRowVis(m_sheepAgeLabel, m_sheepAgeSpin, cfg.showSheepAge);
    m_benedictionCheck->setVisible(cfg.showBenediction);
    m_mediTileCheck->setVisible(cfg.showMediTile);
    m_mediCooldownCheck->setVisible(cfg.showMediCooldown);
    m_isFoCheck->setVisible(cfg.showFo);
    m_isHotsCheck->setVisible(cfg.showHots);
    m_isTamedCheck->setVisible(cfg.showTamed);

    if (m_linkSkillsCheck->isChecked()) {
        syncSkillsFromStats();
    }
    runSimulation();
}

void GrinderWidget::onActionChanged(int index) {
    if (!m_initialized) return;
    auto mode = static_cast<ActionMode>(index);
    populateDifficultyPresets(mode);
    updateModeVisibility();
}

void GrinderWidget::populateDifficultyPresets(ActionMode mode) {
    if (!m_difficultyCombo || !m_difficultySpin) return;

    auto presets = DifficultyProvider::getPresetsForMode(mode);
    m_difficultyCombo->blockSignals(true);
    m_difficultyCombo->clear();
    m_difficultyCombo->addItem("Custom", QVariant());
    for (const auto& [name, diff] : presets) {
        m_difficultyCombo->addItem(QString("%1 (%2)").arg(name).arg(diff), diff);
    }

    if (!presets.empty()) {
        m_difficultyCombo->setCurrentIndex(1);
        double diff = presets.front().difficulty;
        m_difficultySpin->blockSignals(true);
        m_difficultySpin->setValue(diff);
        m_difficultySpin->blockSignals(false);
    } else {
        m_difficultyCombo->setCurrentIndex(0);
    }
    m_difficultyCombo->blockSignals(false);
}

void GrinderWidget::onDifficultyPresetChanged(int index) {
    if (!m_initialized || !m_difficultyCombo || !m_difficultySpin) return;

    if (index <= 0) {
        return;
    }

    QVariant data = m_difficultyCombo->itemData(index);
    if (data.isValid()) {
        double val = data.toDouble();
        m_difficultySpin->blockSignals(true);
        m_difficultySpin->setValue(val);
        m_difficultySpin->blockSignals(false);
        runSimulation();
    }
}

void GrinderWidget::onDifficultySpinChanged(double val) {
    if (!m_initialized || !m_difficultyCombo || !m_difficultySpin) return;

    int matchIdx = 0;
    for (int i = 1; i < m_difficultyCombo->count(); ++i) {
        if (std::abs(m_difficultyCombo->itemData(i).toDouble() - val) < 0.001) {
            matchIdx = i;
            break;
        }
    }

    m_difficultyCombo->blockSignals(true);
    m_difficultyCombo->setCurrentIndex(matchIdx);
    m_difficultyCombo->blockSignals(false);

    runSimulation();
}

void GrinderWidget::onMobChanged(int) {
    if (!m_initialized) return;
    runSimulation();
}

void GrinderWidget::onLinkSkillsToggled(bool checked) {
    if (!m_initialized) return;
    if (checked) {
        syncSkillsFromStats();
    } else {
        m_linkStatusLabel->setText("Live character sync disabled (manual input)");
        m_linkStatusLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(theme::TEXT_SECONDARY));
    }
}

void GrinderWidget::onSkillsUpdated(const std::vector<skills::SkillStats>& stats) {
    m_latestStats = stats;
    if (m_initialized && m_linkSkillsCheck && m_linkSkillsCheck->isChecked()) {
        syncSkillsFromStats();
    }
}

void GrinderWidget::syncSkillsFromStats() {
    if (!m_initialized || !m_primarySkillSpin) return;
    if (m_latestStats.empty()) {
        m_linkStatusLabel->setText("Waiting for character skill log entries...");
        m_linkStatusLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(theme::STATUS_WARNING));
        return;
    }

    auto findSkill = [this](const std::vector<std::string>& candidates) -> std::pair<std::string, double> {
        for (const auto& cand : candidates) {
            for (const auto& s : m_latestStats) {
                if (QString::compare(QString::fromStdString(s.skill_name), QString::fromStdString(cand), Qt::CaseInsensitive) == 0) {
                    return {s.skill_name, s.current_level};
                }
            }
        }
        return {"", -1.0};
    };

    auto mode = static_cast<ActionMode>(m_actionCombo->currentIndex());
    auto cfg = getModeConfig(mode);

    QStringList linkedInfo;
    auto [pName, pVal] = findSkill(cfg.primaryCandidates);
    if (pVal >= 0.0) {
        m_primarySkillSpin->setValue(pVal);
        linkedInfo << QString("%1: <b>%2</b>").arg(QString::fromStdString(pName)).arg(pVal, 0, 'f', 2);
    }
    if (!cfg.secondaryCandidates.empty()) {
        auto [sName, sVal] = findSkill(cfg.secondaryCandidates);
        if (sVal >= 0.0) {
            m_secondarySkillSpin->setValue(sVal);
            linkedInfo << QString("%1: <b>%2</b>").arg(QString::fromStdString(sName)).arg(sVal, 0, 'f', 2);
        }
    }
    if (!cfg.tertiaryCandidates.empty()) {
        auto [tName, tVal] = findSkill(cfg.tertiaryCandidates);
        if (tVal >= 0.0) {
            m_tertiarySkillSpin->setValue(tVal);
            linkedInfo << QString("%1: <b>%2</b>").arg(QString::fromStdString(tName)).arg(tVal, 0, 'f', 2);
        }
    }

    if (!linkedInfo.isEmpty()) {
        m_linkStatusLabel->setText(QString("🟢 Live: %1").arg(linkedInfo.join(" | ")));
        m_linkStatusLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(theme::ACCENT_MINT));
    } else {
        m_linkStatusLabel->setText("Linked, but current mode skills not yet in character log.");
        m_linkStatusLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(theme::TEXT_SECONDARY));
    }
}

void GrinderWidget::runSimulation() {
    if (!m_initialized || !m_chart || !m_successRateLabel || !m_primarySkillSpin) return;
    GrinderParams p;
    p.mode = static_cast<ActionMode>(m_actionCombo->currentIndex());
    p.skill = m_primarySkillSpin->value();
    p.secondarySkill = m_secondarySkillSpin->value();
    p.tertiarySkill = m_tertiarySkillSpin->value();
    p.secondaryQl = m_toolQlSpin->value();
    p.itemQl = m_toolQlSpin->value();
    p.difficulty = m_difficultySpin->value();
    p.benediction = m_benedictionCheck->isChecked();
    p.materialQl = m_materialQlSpin->value();
    p.startQl = m_startQlSpin->value();
    p.targetQl = m_targetQlSpin->value();
    p.veinQl = m_veinQlSpin->value();
    p.imbue = m_imbueSpin->value();
    p.rarity = m_raritySpin->value();
    p.rune = m_runeSpin->value();
    p.slope = m_slopeSpin->value();
    p.pathLevel = m_pathLevelSpin->value();
    p.sheepAge = m_sheepAgeSpin->value();
    p.mediTile = m_mediTileCheck->isChecked();
    p.mediCooldown = m_mediCooldownCheck->isChecked();
    p.isFo = m_isFoCheck->isChecked();
    p.isHots = m_isHotsCheck->isChecked();
    p.isTamed = m_isTamedCheck->isChecked();

    if (m_mobCombo->currentIndex() >= 0) {
        p.mobName = m_mobCombo->currentData().toString().toStdString();
    }

    int trials = (p.mode == ActionMode::SmithingSteps) ? 1000 : 5000;
    RollDistribution dist = GrinderEngine::simulate(p, trials);

    QString title = m_actionCombo->currentText() + " Distribution";
    if (p.mode == ActionMode::SmithingSteps) {
        m_successRateLabel->setText("Simulated: <b>1,000 runs</b>");
        m_successRateLabel->setStyleSheet(QString("font-size: 15px; color: %1;").arg(theme::ACCENT_EMERALD));
        m_meanLabel->setText(QString("Average Imp Steps: <b>%1</b>").arg(dist.mean, 0, 'f', 1));
        m_meanLabel->setStyleSheet(QString("color: %1;").arg(theme::ACCENT_MINT));
        m_rangeLabel->setText(QString("Steps Range: [%1 to %2]").arg(dist.min, 0, 'f', 0).arg(dist.max, 0, 'f', 0));
    } else {
        m_successRateLabel->setText(QString("Success Chance: <b>%1%</b>").arg(dist.successRate, 0, 'f', 1));
        m_successRateLabel->setStyleSheet(QString("font-size: 15px; color: %1;")
            .arg(dist.successRate >= 50.0 ? theme::STATUS_SUCCESS : theme::STATUS_DANGER));
        m_meanLabel->setText(QString("Expected Mean: <b>%1</b>").arg(dist.mean, 0, 'f', 2));
        m_meanLabel->setStyleSheet(QString("color: %1;").arg(theme::ACCENT_MINT));
        m_rangeLabel->setText(QString("Value Range: [%1 to %2]").arg(dist.min, 0, 'f', 1).arg(dist.max, 0, 'f', 1));
    }
    m_rangeLabel->setStyleSheet(QString("color: %1;").arg(theme::TEXT_SECONDARY));

    m_chart->setDistribution(dist, title);
}

} // namespace tools
