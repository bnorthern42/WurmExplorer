#pragma once

#include <QWidget>
#include <memory>
#include <vector>
#include "../../models/AnnotationStore.hpp"
#include "../../core/Config.hpp"

class QLineEdit;
class QComboBox;
class QCheckBox;
class QListWidget;
class QLabel;

namespace treasure::ui {

class MapDataPanel : public QWidget {
    Q_OBJECT

public:
    explicit MapDataPanel(std::shared_ptr<treasure::models::AnnotationStore> annotStore, QWidget* parent = nullptr);
    ~MapDataPanel() override = default;

    void setContext(std::shared_ptr<treasure::core::ServerConfig> config);
    std::vector<treasure::models::Annotation> getVisibleMapData() const;

signals:
    void dataChanged();
    void focusRequested(float tx, float ty);

private slots:
    void refreshList();
    void onSelectionChanged(int currentRow);

private:
    void setupUi();
    bool matchesFilters(const treasure::models::Annotation& ann) const;

    std::shared_ptr<treasure::models::AnnotationStore> annotStore;
    std::shared_ptr<treasure::core::ServerConfig> currentConfig;
    
    std::vector<treasure::models::Annotation> filteredItems;

    QLineEdit* searchEdit;
    QComboBox* kindCb;
    QComboBox* sheetCb;
    QLabel* countLabel;
    QListWidget* itemList;

    QCheckBox* showDeedsChk;
    QCheckBox* showTowersChk;
    QCheckBox* showResourcesChk;
    QCheckBox* showSpecialChk;
    QCheckBox* showHighwaysChk;
    QCheckBox* showBridgesChk;
    QCheckBox* showTunnelsChk;
    QCheckBox* showCanalsChk;
};

} // namespace treasure::ui
