#include "ArtifactPanel.hpp"
#include "../ThemeTokens.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QListWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStringList>
#include "../../models/ArtifactStore.hpp"

namespace treasure {
namespace ui {

ArtifactPanel::ArtifactPanel(std::shared_ptr<models::ArtifactStore> store, QWidget* parent)
    : QWidget(parent), artifactStore(store) {
    
    artifactNames = {
        "Staff of Provis", "Rod of Beguiling", "Orb of Doom",
        "Chalice of Incarnation", "Libram of the Night", 
        "Sword of Magran", "Eye of Vynora", "Nyn's Ear",
        "Hammer of Fo", "Vanguard", "Ear of Fo", "Finger of Vynora"
    };
    if (!artifactNames.empty()) {
        selectedArtifactName = artifactNames.front();
    }
    
    setupUi();
}

void ArtifactPanel::setupUi() {
    casterLabel = new QLabel("Caster: (not set)");
    casterLabel->setStyleSheet(QString("font-weight: bold; color: %1;").arg(theme::TEXT_PRIMARY));
    helperLabel = new QLabel("Place the caster, then add cast clues.");
    helperLabel->setWordWrap(true);
    selectionLabel = new QLabel("Selected artifact: none");
    selectionLabel->setStyleSheet(QString("font-weight: bold; color: %1;").arg(theme::ACCENT_MINT));
    
    toolGroup = new QButtonGroup(this);
    panBtn = new QRadioButton("Pan");
    setCasterBtn = new QRadioButton("Set Caster");
    panBtn->setChecked(true);
    toolGroup->addButton(panBtn);
    toolGroup->addButton(setCasterBtn);
    
    artifactList = new QListWidget();
    clueList = new QListWidget();
    connect(artifactList, &QListWidget::currentRowChanged, this, &ArtifactPanel::onArtifactSelect);
    connect(clueList, &QListWidget::currentRowChanged, this, &ArtifactPanel::onClueSelect);
    
    facingCb = new QComboBox();
    facingCb->addItems({"N", "NE", "E", "SE", "S", "SW", "W", "NW"});
    facingCb->setCurrentText("N");
    
    bandCb = new QComboBox();
    bandCb->addItems({
        "0-19 tiles", "20-49 tiles", "50-99 tiles", "100-199 tiles", 
        "200-499 tiles", "500-999 tiles", "1000-1999 tiles", "2000+ tiles"
    });
    bandCb->setCurrentText("20-49 tiles");
    
    coneWidthEdit = new QLineEdit("120");
    rawLogEdit = new QLineEdit();
    
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(10, 10, 10, 10);
    
    QGroupBox* tools = new QGroupBox("Artifact Tools");
    QVBoxLayout* toolsLayout = new QVBoxLayout(tools);
    QHBoxLayout* toolRow = new QHBoxLayout();
    toolRow->addWidget(panBtn);
    toolRow->addWidget(setCasterBtn);
    toolRow->addStretch(1);
    toolsLayout->addLayout(toolRow);
    toolsLayout->addWidget(casterLabel);
    QLabel* note = new QLabel("Assumes north or up by default. Use facing to model 'in front of you'.");
    note->setStyleSheet(QString("color: %1;").arg(theme::TEXT_SECONDARY)); // muted text
    note->setWordWrap(true);
    toolsLayout->addWidget(note);
    outer->addWidget(tools);
    
    QGroupBox* browser = new QGroupBox("Artifacts");
    QGridLayout* browserLayout = new QGridLayout(browser);
    browserLayout->addWidget(selectionLabel, 0, 0, 1, 2);
    browserLayout->addWidget(artifactList, 1, 0);
    browserLayout->addWidget(clueList, 1, 1);
    outer->addWidget(browser, 1);
    
    QGroupBox* entry = new QGroupBox("Add Cast");
    QFormLayout* entryLayout = new QFormLayout(entry);
    entryLayout->addRow("Facing", facingCb);
    entryLayout->addRow("Band", bandCb);
    entryLayout->addRow("Cone °", coneWidthEdit);
    entryLayout->addRow("Raw log", rawLogEdit);
    
    QGridLayout* btns = new QGridLayout();
    QPushButton* addCastBtn = new QPushButton("Add Cast");
    addCastBtn->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; border-radius: 4px; padding: 6px;").arg(theme::ACCENT_EMERALD, theme::TEXT_ON_ACCENT));
    connect(addCastBtn, &QPushButton::clicked, this, &ArtifactPanel::onAddCast);
    QPushButton* addLogBtn = new QPushButton("Add From Log");
    connect(addLogBtn, &QPushButton::clicked, this, &ArtifactPanel::onAddFromLog);
    QPushButton* delBtn = new QPushButton("Delete Clue");
    delBtn->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; border-radius: 4px; padding: 6px;").arg(theme::STATUS_DANGER, theme::TEXT_PRIMARY));
    connect(delBtn, &QPushButton::clicked, this, &ArtifactPanel::onDeleteClue);
    QPushButton* clearOneBtn = new QPushButton("Clear Artifact");
    connect(clearOneBtn, &QPushButton::clicked, this, &ArtifactPanel::onClearArtifact);
    QPushButton* clearAllBtn = new QPushButton("Clear All");
    connect(clearAllBtn, &QPushButton::clicked, this, &ArtifactPanel::onClearAll);
    
    btns->addWidget(addCastBtn, 0, 0);
    btns->addWidget(addLogBtn, 0, 1);
    btns->addWidget(delBtn, 1, 0);
    btns->addWidget(clearOneBtn, 1, 1);
    btns->addWidget(clearAllBtn, 2, 0, 1, 2);
    
    entryLayout->addRow(btns);
    outer->addWidget(entry);
    
    QGroupBox* notesGroup = new QGroupBox("Notes");
    QVBoxLayout* notesLayout = new QVBoxLayout(notesGroup);
    notesLayout->addWidget(helperLabel);
    QLabel* footer = new QLabel("Selected artifact uses stippled fill. Others are outline only.");
    footer->setStyleSheet(QString("color: %1;").arg(theme::TEXT_SECONDARY));
    notesLayout->addWidget(footer);
    outer->addWidget(notesGroup);
}

void ArtifactPanel::setContext(std::shared_ptr<core::ServerConfig> cfg) {
    currentCfg = cfg;
    selectedClueIndex = -1;
    refreshCasterLabel();
    refreshArtifactList();
    refreshClueList();
    emit dataChanged();
}

void ArtifactPanel::refreshCasterLabel() {
    if (!currentCfg) {
        casterLabel->setText("Caster: (not set)");
        return;
    }
    treasure::models::Point caster;
    if (!artifactStore->getCaster(currentCfg->name, caster)) {
        casterLabel->setText("Caster: (not set)");
    } else {
        casterLabel->setText(QString("Caster: (%1, %2)").arg(caster.x, 0, 'f', 1).arg(caster.y, 0, 'f', 1));
    }
}

void ArtifactPanel::refreshArtifactList() {
    artifactList->blockSignals(true);
    artifactList->clear();
    
    int selectedIdx = 0;
    
    for (size_t i = 0; i < artifactNames.size(); ++i) {
        const auto& name = artifactNames[i];
        int count = currentCfg ? artifactStore->getClues(currentCfg->name, name).size() : 0;
        artifactList->addItem(QString::fromStdString(name + " (" + std::to_string(count) + ")"));
        if (name == selectedArtifactName) {
            selectedIdx = static_cast<int>(i);
        }
    }
    
    artifactList->blockSignals(false);
    if (!artifactNames.empty()) {
        artifactList->setCurrentRow(selectedIdx);
        selectionLabel->setText(QString("Selected artifact: %1").arg(QString::fromStdString(selectedArtifactName)));
    }
}

void ArtifactPanel::refreshClueList() {
    clueList->blockSignals(true);
    clueList->clear();
    selectedClueIndex = -1;
    
    if (currentCfg) {
        auto clues = artifactStore->getClues(currentCfg->name, selectedArtifactName);
        for (size_t i = 0; i < clues.size(); ++i) {
            const auto& clue = clues[i];
            QString text = QString("#%1  %2  facing %3  @ (%4, %5)")
                .arg(i + 1)
                .arg(QString::fromStdString(clue.band_label))
                .arg(QString::fromStdString(clue.facing))
                .arg(clue.caster_tile.x, 0, 'f', 1)
                .arg(clue.caster_tile.y, 0, 'f', 1);
            clueList->addItem(text);
        }
    }
    
    clueList->blockSignals(false);
}

void ArtifactPanel::onArtifactSelect(int idx) {
    if (idx >= 0 && idx < static_cast<int>(artifactNames.size())) {
        selectedArtifactName = artifactNames[idx];
        selectionLabel->setText(QString("Selected artifact: %1").arg(QString::fromStdString(selectedArtifactName)));
        refreshClueList();
        emit dataChanged();
    }
}

void ArtifactPanel::onClueSelect(int idx) {
    selectedClueIndex = (idx >= 0) ? idx : -1;
}

float ArtifactPanel::currentConeWidth() const {
    bool ok;
    float value = coneWidthEdit->text().toFloat(&ok);
    if (!ok) value = 120.0f;
    return std::max(30.0f, std::min(360.0f, value));
}

void ArtifactPanel::onAddCast() {
    if (!currentCfg) return;
    
    treasure::models::Point caster;
    if (!artifactStore->getCaster(currentCfg->name, caster)) {
        helperLabel->setText("Set the caster position first.");
        return;
    }
    
    models::ArtifactClue clue;
    clue.artifact = selectedArtifactName;
    clue.caster_tile = caster;
    QString facingText = facingCb->currentText().trimmed();
    clue.facing = facingText.isEmpty() ? "N" : facingText.toStdString();
    QString bandText = bandCb->currentText().trimmed();
    clue.band_label = bandText.isEmpty() ? "20-49 tiles" : bandText.toStdString();
    clue.cone_width_deg = currentConeWidth();
    
    artifactStore->addClue(currentCfg->name, clue);
    helperLabel->setText("Artifact cast clue added.");
    refreshArtifactList();
    refreshClueList();
    emit dataChanged();
}

void ArtifactPanel::onAddFromLog() {
    if (!currentCfg) return;
    
    treasure::models::Point caster;
    if (!artifactStore->getCaster(currentCfg->name, caster)) {
        helperLabel->setText("Set the caster position first.");
        return;
    }
    
    // Simple mock for parsing log since the regex in python isn't trivial to port entirely inline.
    // Ideally this uses a C++ parser for the log lines, but for now we'll just check if it matches a band.
    QString logText = rawLogEdit->text().trimmed();
    if (logText.isEmpty()) {
        helperLabel->setText("Could not parse artifact name and distance phrase from log line.");
        return;
    }
    
    // For now we just pretend we couldn't parse if we don't implement the full regex yet.
    // In a real port we'd use std::regex here to extract the name and distance phrase.
    // Let's implement a rudimentary regex search if possible, or just skip it for this stage.
    // ...
    
    helperLabel->setText("Add from log currently requires regex implementation.");
}

void ArtifactPanel::onDeleteClue() {
    if (!currentCfg || selectedClueIndex < 0) {
        helperLabel->setText("Select a clue to delete.");
        return;
    }
    artifactStore->removeClue(currentCfg->name, selectedArtifactName, selectedClueIndex);
    helperLabel->setText("Clue deleted.");
    refreshArtifactList();
    refreshClueList();
    emit dataChanged();
}

void ArtifactPanel::onClearArtifact() {
    if (!currentCfg) return;
    artifactStore->clearArtifact(currentCfg->name, selectedArtifactName);
    helperLabel->setText(QString("Cleared clues for %1.").arg(QString::fromStdString(selectedArtifactName)));
    refreshArtifactList();
    refreshClueList();
    emit dataChanged();
}

void ArtifactPanel::onClearAll() {
    if (!currentCfg) return;
    artifactStore->clearServer(currentCfg->name);
    helperLabel->setText("Cleared all artifact clues for this server.");
    refreshArtifactList();
    refreshClueList();
    emit dataChanged();
}

} // namespace ui
} // namespace treasure
