#pragma once

#include <QWidget>
#include <memory>
#include <vector>
#include <map>
#include <string>

#include "../../models/ClusterDrawingStore.hpp"
#include "../../models/ClusterLayout.hpp"
#include "../../core/Config.hpp"
#include "../../engine/CanvasOverlays.hpp"

class QComboBox;
class QLineEdit;
class QListWidget;
class QLabel;
class QPushButton;
class QSpinBox;
class QTextEdit;
class QButtonGroup;

namespace treasure {
namespace ui {

class SailingPanel : public QWidget {
    Q_OBJECT

public:
    explicit SailingPanel(std::shared_ptr<treasure::models::ClusterDrawingStore> drawingStore, QWidget* parent = nullptr);
    ~SailingPanel() override = default;

    void setContext(const std::map<std::string, std::shared_ptr<treasure::core::ServerConfig>>& allConfigs, const std::string& currentMapType, const std::map<std::string, std::string>& mapImages);
    
    // Will return the stitched cluster map QImage
    QImage getDisplayImage() const;
    treasure::ui::CanvasOverlays getOverlays() const;

signals:
    void dataChanged();
    void imageChanged();

private slots:
    void refreshObjectList();
    void onPlanChanged();
    void onToolChanged();
    void pickColor();
    void newPlan();
    void resetEditor();
    void onObjectSelected(int currentRow);
    void deleteCurrent();
    void removeLastItem();
    void deletePlan();
    void saveCurrent();
    void recomputeCrossing();

private:
    void setupUi();
    void refreshPlanNames();
    void refreshLayerFilter();
    void buildOverlays();

    std::shared_ptr<treasure::models::ClusterDrawingStore> drawingStore;
    std::map<std::string, std::shared_ptr<treasure::core::ServerConfig>> cfgByName;
    std::string currentMapType = "terrain";
    std::map<std::string, std::string> currentMapImages;

    treasure::models::ClusterLayout layoutModel;
    std::optional<treasure::models::ClusterRenderState> renderState;

    // UI elements
    QComboBox* modeCb;
    QComboBox* sourceServerCb;
    QComboBox* destServerCb;
    QComboBox* edgeCb;
    QLabel* pickLabel;
    QLabel* arrivalLabel;

    QComboBox* planCb;
    QComboBox* layerFilterCb;

    QButtonGroup* toolGroup;
    QLineEdit* objectNameEdit;
    QLineEdit* layerEdit;
    QPushButton* colorBtn;
    QSpinBox* widthSpin;
    QTextEdit* textEdit;

    QListWidget* objectList;
    QLabel* selectionLabel;
    QLabel* posLabel;
    QLabel* countLabel;
    QLabel* helperLabel;

    QPushButton* saveBtn;
    QPushButton* newBtn;
    QPushButton* removeLastItemBtn;
    QPushButton* deleteBtn;
    
    std::string currentObjectId;
    std::vector<std::pair<float, float>> pendingPoints;
    std::vector<treasure::models::ClusterDrawingObject> filteredObjects;
    std::optional<treasure::models::SailingResult> currentResult;
    
    treasure::ui::CanvasOverlays cachedOverlays;
};

} // namespace ui
} // namespace treasure
