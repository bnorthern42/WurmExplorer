#pragma once

#include <QWidget>
#include <memory>
#include "../../models/ArtifactStore.hpp"
#include "../../core/Config.hpp"

class QListWidget;
class QComboBox;
class QLineEdit;
class QLabel;
class QRadioButton;
class QButtonGroup;
class QPushButton;

namespace treasure {
namespace ui {

class ArtifactPanel : public QWidget {
    Q_OBJECT
public:
    explicit ArtifactPanel(std::shared_ptr<models::ArtifactStore> store, QWidget* parent = nullptr);
    
    void setContext(std::shared_ptr<core::ServerConfig> cfg);

signals:
    void dataChanged();

private slots:
    void onArtifactSelect(int idx);
    void onClueSelect(int idx);
    void onAddCast();
    void onAddFromLog();
    void onDeleteClue();
    void onClearArtifact();
    void onClearAll();

private:
    void setupUi();
    void refreshCasterLabel();
    void refreshArtifactList();
    void refreshClueList();
    
    float currentConeWidth() const;

    std::shared_ptr<models::ArtifactStore> artifactStore;
    std::shared_ptr<core::ServerConfig> currentCfg;
    
    std::string selectedArtifactName;
    int selectedClueIndex = -1;
    std::vector<std::string> artifactNames;
    
    QLabel* casterLabel;
    QLabel* helperLabel;
    QLabel* selectionLabel;
    
    QButtonGroup* toolGroup;
    QRadioButton* panBtn;
    QRadioButton* setCasterBtn;
    
    QListWidget* artifactList;
    QListWidget* clueList;
    
    QComboBox* facingCb;
    QComboBox* bandCb;
    QLineEdit* coneWidthEdit;
    QLineEdit* rawLogEdit;
};

} // namespace ui
} // namespace treasure
