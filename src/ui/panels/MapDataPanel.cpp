#include "MapDataPanel.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>
#include <QLabel>
#include <QString>
#include <algorithm>

namespace treasure::ui {

MapDataPanel::MapDataPanel(std::shared_ptr<treasure::models::AnnotationStore> annotStore, QWidget* parent)
    : QWidget(parent), annotStore(std::move(annotStore)) {
    setupUi();
}

void MapDataPanel::setupUi() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(10, 10, 10, 10);
    outer->setSpacing(10);

    // Filters
    auto* filters = new QGroupBox("Map Data Filters", this);
    auto* filtersLayout = new QFormLayout(filters);
    
    searchEdit = new QLineEdit(this);
    filtersLayout->addRow("Search", searchEdit);

    kindCb = new QComboBox(this);
    kindCb->addItems({"All", "deed", "guard_tower", "resource", "special", "highway", "bridge", "tunnel", "canal"});
    filtersLayout->addRow("Kind", kindCb);

    sheetCb = new QComboBox(this);
    sheetCb->addItems({"All", "Deeds", "Highways", "Bridges", "Tunnels", "Resources", "Special"});
    filtersLayout->addRow("Sheet", sheetCb);

    outer->addWidget(filters);

    // Visible Layers
    auto* layerBox = new QGroupBox("Visible Layers", this);
    auto* layerLayout = new QGridLayout(layerBox);

    showDeedsChk = new QCheckBox("Deeds", this);
    showTowersChk = new QCheckBox("Guard Towers", this);
    showResourcesChk = new QCheckBox("Resources", this);
    showSpecialChk = new QCheckBox("Special", this);
    showHighwaysChk = new QCheckBox("Highways", this);
    showBridgesChk = new QCheckBox("Bridges", this);
    showTunnelsChk = new QCheckBox("Tunnels", this);
    showCanalsChk = new QCheckBox("Canals", this);

    std::vector<QCheckBox*> toggles = {
        showDeedsChk, showTowersChk, showResourcesChk, showSpecialChk,
        showHighwaysChk, showBridgesChk, showTunnelsChk, showCanalsChk
    };

    for (size_t i = 0; i < toggles.size(); ++i) {
        toggles[i]->setChecked(true);
        layerLayout->addWidget(toggles[i], i / 2, i % 2);
        connect(toggles[i], &QCheckBox::checkStateChanged, this, &MapDataPanel::refreshList);
    }
    outer->addWidget(layerBox);

    // Imported Map Data
    auto* browser = new QGroupBox("Imported Map Data", this);
    auto* browserLayout = new QVBoxLayout(browser);
    
    itemList = new QListWidget(this);
    browserLayout->addWidget(itemList, 1);

    auto* footer = new QHBoxLayout();
    countLabel = new QLabel("0 shown", this);
    footer->addWidget(countLabel);

    auto* note = new QLabel("Select to center on map.", this);
    note->setStyleSheet("color: gray;");
    footer->addWidget(note, 1, Qt::AlignRight);
    
    browserLayout->addLayout(footer);
    outer->addWidget(browser, 1);

    // Connections
    connect(itemList, &QListWidget::currentRowChanged, this, &MapDataPanel::onSelectionChanged);
    connect(searchEdit, &QLineEdit::textChanged, this, &MapDataPanel::refreshList);
    connect(kindCb, &QComboBox::currentIndexChanged, this, &MapDataPanel::refreshList);
    connect(sheetCb, &QComboBox::currentIndexChanged, this, &MapDataPanel::refreshList);
}

void MapDataPanel::setContext(std::shared_ptr<treasure::core::ServerConfig> config) {
    currentConfig = config;
    refreshList();
}

bool MapDataPanel::matchesFilters(const treasure::models::Annotation& ann) const {
    if (ann.source != "import") return false;
    if (!currentConfig || ann.server != currentConfig->name) return false;

    QString kind = QString::fromStdString(ann.imported_kind.empty() ? ann.type : ann.imported_kind).toLower();

    // Check layer visibility
    if (kind == "deed" && !showDeedsChk->isChecked()) return false;
    if (kind == "guard_tower" && !showTowersChk->isChecked()) return false;
    if (kind == "resource" && !showResourcesChk->isChecked()) return false;
    if (kind == "special" && !showSpecialChk->isChecked()) return false;
    if (kind == "highway" && !showHighwaysChk->isChecked()) return false;
    if (kind == "bridge" && !showBridgesChk->isChecked()) return false;
    if (kind == "tunnel" && !showTunnelsChk->isChecked()) return false;
    if (kind == "canal" && !showCanalsChk->isChecked()) return false;

    QString wantedKind = kindCb->currentText().trimmed();
    if (!wantedKind.isEmpty() && wantedKind != "All" && kind != wantedKind) return false;

    QString wantedSheet = sheetCb->currentText().trimmed();
    if (!wantedSheet.isEmpty() && wantedSheet != "All" && QString::fromStdString(ann.source_sheet) != wantedSheet) return false;

    QString search = searchEdit->text().trimmed().toLower();
    if (!search.isEmpty()) {
        QString hay = QString::fromStdString(ann.name + "\n" + ann.notes + "\n" + ann.tags + "\n" + ann.category + "\n" + ann.source_sheet).toLower();
        if (!hay.contains(search)) return false;
    }

    return true;
}

void MapDataPanel::refreshList() {
    itemList->blockSignals(true);
    itemList->clear();
    filteredItems.clear();

    if (!currentConfig || !annotStore) {
        countLabel->setText("0 shown");
        itemList->blockSignals(false);
        emit dataChanged();
        return;
    }

    auto imported = annotStore->getImportedByServer(currentConfig->name);
    
    for (const auto& ann : imported) {
        if (!matchesFilters(ann)) continue;
        filteredItems.push_back(ann);

        std::string kind = ann.imported_kind.empty() ? ann.type : ann.imported_kind;
        QString label = QString("[%1] %2").arg(QString::fromStdString(kind), QString::fromStdString(ann.name));
        if (!ann.category.empty()) {
            label += QString(" - %1").arg(QString::fromStdString(ann.category));
        }
        itemList->addItem(label);
    }

    countLabel->setText(QString("%1 shown").arg(filteredItems.size()));
    itemList->blockSignals(false);

    emit dataChanged();
}

void MapDataPanel::onSelectionChanged(int currentRow) {
    if (currentRow < 0 || currentRow >= static_cast<int>(filteredItems.size())) return;
    
    const auto& ann = filteredItems[currentRow];
    if (!ann.points.empty()) {
        emit focusRequested(ann.points[0].x, ann.points[0].y);
    }
}

std::vector<treasure::models::Annotation> MapDataPanel::getVisibleMapData() const {
    return filteredItems;
}

} // namespace treasure::ui
