#pragma once

#include <QDialog>
#include <QString>
#include <memory>

class QLineEdit;
class QComboBox;
class QSpinBox;
class QRadioButton;
class QPushButton;

namespace ui {

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override = default;

    static QString getPlayersDir();
    static QString getActivePlayer();
    static QString getLogMode();
    static QString getCustomLogPath();
    static int getPollIntervalSec();

    static QString resolveEffectiveLogFile();

signals:
    void settingsSaved();

private slots:
    void onBrowsePlayersDir();
    void onBrowseCustomLog();
    void onPlayersDirChanged(const QString& newDir);
    void onSaveClicked();

private:
    void setupUi();
    void loadFromSettings();
    void refreshPlayersDropdown();
    void refreshCustomLogDropdown();

    QLineEdit* m_playersDirEdit = nullptr;
    QPushButton* m_browsePlayersBtn = nullptr;

    QComboBox* m_playerCombo = nullptr;

    QRadioButton* m_autoMonthRadio = nullptr;
    QRadioButton* m_customLogRadio = nullptr;

    QComboBox* m_customLogCombo = nullptr;
    QLineEdit* m_customLogEdit = nullptr;
    QPushButton* m_browseCustomLogBtn = nullptr;

    QSpinBox* m_pollIntervalSpin = nullptr;
};

} // namespace ui
