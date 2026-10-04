#pragma once

#include <QWidget>
#include <QString>
#include <QTimer>
#include "SkillTracker.hpp"

class QTableWidget;
class QLineEdit;
class QLabel;
class QPushButton;

namespace skills {

class SkillsPanel : public QWidget {
    Q_OBJECT

public:
    explicit SkillsPanel(QWidget* parent = nullptr);
    ~SkillsPanel() override = default;

    std::vector<SkillStats> getStats() const;
    double getSkillLevel(const std::string& skillName) const;

signals:
    void skillsUpdated(const std::vector<skills::SkillStats>& stats);

public slots:
    void reloadFile();
    void onSettingsSaved();

private slots:
    void onTimerTick();
    void onSearchChanged(const QString& text);
    void onTogglePauseClicked();
    void onReloadClicked();
    void onSettingsClicked();

private:
    void setupUi();
    void pollFile();
    void updateTable();

    SkillTracker m_tracker;
    QTimer* m_pollTimer = nullptr;

    QString m_currentFilePath;
    int64_t m_lastFileOffset = 0;
    bool m_paused = false;

    // UI elements
    QLabel* m_watchingLabel = nullptr;
    QLabel* m_statusBadge = nullptr;
    QLabel* m_metricsLabel = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    QTableWidget* m_table = nullptr;
    QPushButton* m_pauseBtn = nullptr;
};

} // namespace skills
