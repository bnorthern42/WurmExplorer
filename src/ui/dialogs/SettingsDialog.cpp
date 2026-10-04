#include "SettingsDialog.hpp"
#include "../../features/skills/SkillPathResolver.hpp"
#include "../ThemeTokens.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QPushButton>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QSettings>
#include <filesystem>

namespace ui {

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Wurm Settings & Logs Configuration");
    resize(520, 420);
    setupUi();
    loadFromSettings();
}

void SettingsDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // Group 1: Players & Logs Directory
    auto* dirGroup = new QGroupBox("Wurm Client & Logs Location", this);
    auto* dirLayout = new QFormLayout(dirGroup);

    auto* dirRow = new QHBoxLayout();
    m_playersDirEdit = new QLineEdit(dirGroup);
    m_browsePlayersBtn = new QPushButton("Browse...", dirGroup);
    connect(m_browsePlayersBtn, &QPushButton::clicked, this, &SettingsDialog::onBrowsePlayersDir);
    connect(m_playersDirEdit, &QLineEdit::textChanged, this, &SettingsDialog::onPlayersDirChanged);
    dirRow->addWidget(m_playersDirEdit, 1);
    dirRow->addWidget(m_browsePlayersBtn);
    dirLayout->addRow("Players Directory:", dirRow);

    m_playerCombo = new QComboBox(dirGroup);
    connect(m_playerCombo, &QComboBox::currentIndexChanged, this, [this]() {
        refreshCustomLogDropdown();
    });
    dirLayout->addRow("Active Player:", m_playerCombo);

    mainLayout->addWidget(dirGroup);

    // Group 2: Skill Log Source
    auto* logGroup = new QGroupBox("Skill Log Tracking Mode", this);
    auto* logLayout = new QVBoxLayout(logGroup);

    m_autoMonthRadio = new QRadioButton("Auto-detect current month (_Skills.YYYY-MM.txt)", logGroup);
    m_customLogRadio = new QRadioButton("Use specific log file:", logGroup);

    logLayout->addWidget(m_autoMonthRadio);
    logLayout->addWidget(m_customLogRadio);

    auto* customLogRow = new QHBoxLayout();
    m_customLogCombo = new QComboBox(logGroup);
    connect(m_customLogCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        if (m_customLogCombo->currentIndex() >= 0) {
            QString chosen = m_customLogCombo->currentData().toString();
            if (!chosen.isEmpty()) {
                m_customLogEdit->setText(chosen);
            }
        }
    });

    m_customLogEdit = new QLineEdit(logGroup);
    m_browseCustomLogBtn = new QPushButton("Browse...", logGroup);
    connect(m_browseCustomLogBtn, &QPushButton::clicked, this, &SettingsDialog::onBrowseCustomLog);

    customLogRow->addWidget(m_customLogCombo, 1);
    customLogRow->addWidget(m_browseCustomLogBtn);
    logLayout->addLayout(customLogRow);
    logLayout->addWidget(m_customLogEdit);

    connect(m_autoMonthRadio, &QRadioButton::toggled, this, [this](bool checked) {
        m_customLogCombo->setEnabled(!checked);
        m_customLogEdit->setEnabled(!checked);
        m_browseCustomLogBtn->setEnabled(!checked);
    });

    mainLayout->addWidget(logGroup);

    // Group 3: Polling options
    auto* pollGroup = new QGroupBox("Tracking Options", this);
    auto* pollLayout = new QFormLayout(pollGroup);
    m_pollIntervalSpin = new QSpinBox(pollGroup);
    m_pollIntervalSpin->setRange(1, 60);
    m_pollIntervalSpin->setSuffix(" sec");
    pollLayout->addRow("Live Log Check Interval:", m_pollIntervalSpin);
    mainLayout->addWidget(pollGroup);

    // Buttons
    auto* btnRow = new QHBoxLayout();
    btnRow->addStretch(1);
    auto* saveBtn = new QPushButton("Save Settings", this);
    saveBtn->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; padding: 6px 14px; border-radius: 4px;").arg(treasure::ui::theme::ACCENT_EMERALD, treasure::ui::theme::TEXT_ON_ACCENT));
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSaveClicked);

    auto* cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3; padding: 6px 14px; border-radius: 4px;").arg(treasure::ui::theme::SURFACE_CARD, treasure::ui::theme::TEXT_PRIMARY, treasure::ui::theme::BORDER_MUTED));
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);
}

void SettingsDialog::loadFromSettings() {
    m_playersDirEdit->setText(getPlayersDir());
    refreshPlayersDropdown();

    QString activePlayer = getActivePlayer();
    int idx = m_playerCombo->findText(activePlayer);
    if (idx >= 0) m_playerCombo->setCurrentIndex(idx);

    refreshCustomLogDropdown();

    QString mode = getLogMode();
    if (mode == "custom") {
        m_customLogRadio->setChecked(true);
        m_customLogEdit->setText(getCustomLogPath());
    } else {
        m_autoMonthRadio->setChecked(true);
    }

    m_pollIntervalSpin->setValue(getPollIntervalSec());
}

void SettingsDialog::refreshPlayersDropdown() {
    m_playerCombo->clear();
    std::string dir = m_playersDirEdit->text().trimmed().toStdString();
    auto players = skills::SkillPathResolver::getAvailablePlayers(dir);
    for (const auto& p : players) {
        m_playerCombo->addItem(QString::fromStdString(p));
    }
}

void SettingsDialog::refreshCustomLogDropdown() {
    m_customLogCombo->clear();
    std::string playersDir = m_playersDirEdit->text().trimmed().toStdString();
    std::string player = m_playerCombo->currentText().trimmed().toStdString();
    std::string logsDir = skills::SkillPathResolver::getLogsDir(playersDir, player);

    auto logFiles = skills::SkillPathResolver::getAvailableLogFiles(logsDir);
    for (const auto& file : logFiles) {
        QString fullPath = QString::fromStdString(logsDir + "/" + file);
        m_customLogCombo->addItem(QString::fromStdString(file), fullPath);
    }
}

void SettingsDialog::onBrowsePlayersDir() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Wurm Players Directory", m_playersDirEdit->text());
    if (!dir.isEmpty()) {
        m_playersDirEdit->setText(dir);
    }
}

void SettingsDialog::onBrowseCustomLog() {
    std::string playersDir = m_playersDirEdit->text().trimmed().toStdString();
    std::string player = m_playerCombo->currentText().trimmed().toStdString();
    QString startDir = QString::fromStdString(skills::SkillPathResolver::getLogsDir(playersDir, player));

    QString file = QFileDialog::getOpenFileName(this, "Select Skill Log File", startDir, "Skill Logs (*_Skills.*.txt *.txt);;All Files (*)");
    if (!file.isEmpty()) {
        m_customLogEdit->setText(file);
    }
}

void SettingsDialog::onPlayersDirChanged(const QString&) {
    refreshPlayersDropdown();
    refreshCustomLogDropdown();
}

void SettingsDialog::onSaveClicked() {
    QSettings settings("WurmExplorer", "Settings");
    settings.setValue("wurmPlayersDir", m_playersDirEdit->text().trimmed());
    settings.setValue("wurmActivePlayer", m_playerCombo->currentText().trimmed());
    settings.setValue("wurmLogMode", m_customLogRadio->isChecked() ? "custom" : "current_month");
    settings.setValue("wurmCustomLogPath", m_customLogEdit->text().trimmed());
    settings.setValue("wurmPollIntervalSec", m_pollIntervalSpin->value());

    emit settingsSaved();
    accept();
}

QString SettingsDialog::getPlayersDir() {
    QSettings settings("WurmExplorer", "Settings");
    QString def = QString::fromStdString(skills::SkillPathResolver::getDefaultPlayersDir());
    if (settings.contains("wurmPlayersDir")) {
        return settings.value("wurmPlayersDir").toString();
    }
    QSettings legacy("WurmLocator", "Settings");
    return legacy.value("wurmPlayersDir", def).toString();
}

QString SettingsDialog::getActivePlayer() {
    QSettings settings("WurmExplorer", "Settings");
    if (settings.contains("wurmActivePlayer")) {
        return settings.value("wurmActivePlayer").toString();
    }
    QSettings legacy("WurmLocator", "Settings");
    return legacy.value("wurmActivePlayer", "").toString();
}

QString SettingsDialog::getLogMode() {
    QSettings settings("WurmExplorer", "Settings");
    if (settings.contains("wurmLogMode")) {
        return settings.value("wurmLogMode").toString();
    }
    QSettings legacy("WurmLocator", "Settings");
    return legacy.value("wurmLogMode", "current_month").toString();
}

QString SettingsDialog::getCustomLogPath() {
    QSettings settings("WurmExplorer", "Settings");
    if (settings.contains("wurmCustomLogPath")) {
        return settings.value("wurmCustomLogPath").toString();
    }
    QSettings legacy("WurmLocator", "Settings");
    return legacy.value("wurmCustomLogPath", "").toString();
}

int SettingsDialog::getPollIntervalSec() {
    QSettings settings("WurmExplorer", "Settings");
    if (settings.contains("wurmPollIntervalSec")) {
        return settings.value("wurmPollIntervalSec").toInt();
    }
    QSettings legacy("WurmLocator", "Settings");
    return legacy.value("wurmPollIntervalSec", 1).toInt();
}

QString SettingsDialog::resolveEffectiveLogFile() {
    if (getLogMode() == "custom") {
        QString custom = getCustomLogPath();
        if (!custom.isEmpty() && std::filesystem::exists(custom.toStdString())) {
            return custom;
        }
    }
    std::string path = skills::SkillPathResolver::resolveCurrentLogPath(getPlayersDir().toStdString(), getActivePlayer().toStdString());
    return QString::fromStdString(path);
}

} // namespace ui
