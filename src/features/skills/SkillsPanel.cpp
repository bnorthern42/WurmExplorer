#include "SkillsPanel.hpp"
#include "../../ui/dialogs/SettingsDialog.hpp"
#include "../../ui/ThemeTokens.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>

namespace skills {

using namespace treasure::ui;

SkillsPanel::SkillsPanel(QWidget* parent)
    : QWidget(parent) {
    setupUi();

    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &SkillsPanel::onTimerTick);

    int intervalSec = ui::SettingsDialog::getPollIntervalSec();
    m_pollTimer->start(intervalSec * 1000);

    reloadFile();
}

void SkillsPanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    // Header row
    auto* headerRow = new QHBoxLayout();
    headerRow->setSpacing(6);

    m_statusBadge = new QLabel("● LIVE", this);
    m_statusBadge->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px;").arg(theme::STATUS_SUCCESS));

    m_watchingLabel = new QLabel("Watching: Initializing...", this);
    m_watchingLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(theme::TEXT_SECONDARY));

    headerRow->addWidget(m_statusBadge);
    headerRow->addWidget(m_watchingLabel, 1);

    mainLayout->addLayout(headerRow);

    // Metrics summary strip
    m_metricsLabel = new QLabel("Total Gained: +0.0000 | Active Skills: 0", this);
    m_metricsLabel->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3; padding: 6px 10px; border-radius: 4px; font-weight: 500;").arg(theme::SURFACE_CARD, theme::ACCENT_MINT, theme::BORDER_MUTED));
    mainLayout->addWidget(m_metricsLabel);

    // Search filter
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("🔍 Filter skills by name...");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &SkillsPanel::onSearchChanged);
    mainLayout->addWidget(m_searchEdit);

    // Table view
    m_table = new QTableWidget(this);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({
        "Skill", "Level", "Total Gain", "Past 15m", "Past 1h", "Rate/Hour", "Count"
    });

    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    for (int c = 1; c < 7; ++c) {
        m_table->horizontalHeader()->setSectionResizeMode(c, QHeaderView::ResizeToContents);
    }
    m_table->verticalHeader()->setVisible(false);
    m_table->setSortingEnabled(true);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setStyleSheet(QString(R"(
        QTableWidget {
            background-color: %1;
            alternate-background-color: %2;
            gridline-color: %3;
            color: %4;
            border: 1px solid %3;
            border-radius: 6px;
        }
        QHeaderView::section {
            background-color: %2;
            color: %5;
            font-weight: bold;
            padding: 4px;
            border: 1px solid %3;
        }
    )").arg(theme::SURFACE_CARD, theme::SURFACE_DARK, theme::BORDER_MUTED, theme::TEXT_PRIMARY, theme::ACCENT_MINT));

    mainLayout->addWidget(m_table, 1);
}

void SkillsPanel::onSettingsSaved() {
    int intervalSec = ui::SettingsDialog::getPollIntervalSec();
    m_pollTimer->setInterval(intervalSec * 1000);
    reloadFile();
}

void SkillsPanel::onSearchChanged(const QString&) {
    updateTable();
}

void SkillsPanel::onTimerTick() {
    pollFile();
}

void SkillsPanel::reloadFile() {
    m_tracker.reset();
    m_lastFileOffset = 0;
    m_currentFilePath = ui::SettingsDialog::resolveEffectiveLogFile();

    QString player = ui::SettingsDialog::getActivePlayer();
    std::filesystem::path p(m_currentFilePath.toStdString());

    if (m_currentFilePath.isEmpty() || !std::filesystem::exists(p)) {
        if (player.isEmpty()) {
            m_watchingLabel->setText("No player or log file selected (Configure in ⚙ Settings)");
        } else {
            m_watchingLabel->setText(QString("Player: %1 | ⚠ Log not found: %2 (Configure in ⚙ Settings)").arg(player).arg(QString::fromStdString(p.filename().string())));
        }
        updateTable();
        return;
    }

    m_watchingLabel->setText(QString("Player: %1 | 📄 %2").arg(player).arg(QString::fromStdString(p.filename().string())));

    std::ifstream file(m_currentFilePath.toStdString());
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            m_tracker.processLine(line);
        }
        file.clear();
        m_lastFileOffset = static_cast<int64_t>(file.tellg());
    }

    updateTable();
}

void SkillsPanel::pollFile() {
    if (m_paused) return;

    QString currentConfigured = ui::SettingsDialog::resolveEffectiveLogFile();
    if (currentConfigured != m_currentFilePath) {
        reloadFile();
        return;
    }

    std::string stdPath = m_currentFilePath.toStdString();
    if (!std::filesystem::exists(stdPath)) return;

    std::error_code ec;
    auto currentSize = static_cast<int64_t>(std::filesystem::file_size(stdPath, ec));
    if (ec) return;

    if (currentSize > m_lastFileOffset) {
        std::ifstream file(stdPath);
        if (file.is_open()) {
            file.seekg(m_lastFileOffset);
            std::string line;
            while (std::getline(file, line)) {
                m_tracker.processLine(line);
            }
            file.clear();
            m_lastFileOffset = static_cast<int64_t>(file.tellg());
            updateTable();
        }
    }
}

void SkillsPanel::updateTable() {
    auto stats = m_tracker.getStats();
    QString filter = m_searchEdit->text().trimmed().toLower();

    double totalGainedAll = 0.0;
    int active15mCount = 0;
    std::string topSkill;
    double topGain = 0.0;

    for (const auto& s : stats) {
        totalGainedAll += s.total_gain;
        if (s.past_15m_gain > 0.000001) active15mCount++;
        if (s.total_gain > topGain) {
            topGain = s.total_gain;
            topSkill = s.skill_name;
        }
    }

    std::ostringstream metricSs;
    metricSs << std::fixed << std::setprecision(4);
    metricSs << "Total Gained: +" << totalGainedAll
             << " | Active (15m): " << active15mCount
             << " | Top: " << (topSkill.empty() ? "None" : topSkill)
             << " (+" << topGain << ")";
    m_metricsLabel->setText(QString::fromStdString(metricSs.str()));

    m_table->setSortingEnabled(false);
    m_table->setRowCount(0);

    for (const auto& s : stats) {
        QString name = QString::fromStdString(s.skill_name);
        if (!filter.isEmpty() && !name.toLower().contains(filter)) {
            continue;
        }

        int row = m_table->rowCount();
        m_table->insertRow(row);

        bool isActive15m = (s.past_15m_gain > 0.000001);

        auto createItem = [isActive15m](const QString& text, double numVal = -1.0) {
            auto* item = new QTableWidgetItem();
            if (numVal >= 0.0) {
                item->setData(Qt::DisplayRole, text);
                item->setData(Qt::UserRole, numVal);
            } else {
                item->setText(text);
            }
            if (isActive15m) {
                item->setForeground(QColor(theme::ACCENT_MINT)); // Highlight recent gains in mint
            }
            return item;
        };

        char buf[64];

        // 0: Skill
        m_table->setItem(row, 0, createItem(name));

        // 1: Level
        std::snprintf(buf, sizeof(buf), "%.4f", s.current_level);
        m_table->setItem(row, 1, createItem(buf, s.current_level));

        // 2: Total Gain
        std::snprintf(buf, sizeof(buf), "+%.4f", s.total_gain);
        m_table->setItem(row, 2, createItem(buf, s.total_gain));

        // 3: Past 15m
        std::snprintf(buf, sizeof(buf), "+%.4f", s.past_15m_gain);
        m_table->setItem(row, 3, createItem(buf, s.past_15m_gain));

        // 4: Past 1h
        std::snprintf(buf, sizeof(buf), "+%.4f", s.past_60m_gain);
        m_table->setItem(row, 4, createItem(buf, s.past_60m_gain));

        // 5: Rate/Hour
        std::snprintf(buf, sizeof(buf), "%.4f/h", s.rate_per_hour);
        m_table->setItem(row, 5, createItem(buf, s.rate_per_hour));

        // 6: Gains Count
        m_table->setItem(row, 6, createItem(QString::number(s.gain_count), static_cast<double>(s.gain_count)));
    }

    m_table->setSortingEnabled(true);
    emit skillsUpdated(stats);
}

std::vector<SkillStats> SkillsPanel::getStats() const {
    return m_tracker.getStats();
}

double SkillsPanel::getSkillLevel(const std::string& skillName) const {
    const auto* stat = m_tracker.getSkillStats(skillName);
    return stat ? stat->current_level : 0.0;
}

} // namespace skills
