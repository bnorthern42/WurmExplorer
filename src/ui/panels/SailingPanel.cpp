#include "SailingPanel.hpp"
#include "../ThemeTokens.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QButtonGroup>
#include <QFont>
#include <QColorDialog>
#include <QUuid>

namespace treasure {
namespace ui {

SailingPanel::SailingPanel(std::shared_ptr<treasure::models::ClusterDrawingStore> drawingStore, QWidget* parent)
    : QWidget(parent), drawingStore(std::move(drawingStore)) {
    setupUi();
}

void SailingPanel::setupUi() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(10, 10, 10, 10);
    outer->setSpacing(10);

    // Sailing Nav Box
    auto* navBox = new QGroupBox("Sailing", this);
    auto* navLayout = new QFormLayout(navBox);
    
    modeCb = new QComboBox(this);
    modeCb->addItems({"regular", "plot_course"});
    navLayout->addRow("Mode", modeCb);
    
    sourceServerCb = new QComboBox(this);
    navLayout->addRow("Source", sourceServerCb);
    
    destServerCb = new QComboBox(this);
    navLayout->addRow("Destination", destServerCb);
    
    edgeCb = new QComboBox(this);
    edgeCb->addItems({"north", "east", "south", "west"});
    navLayout->addRow("Exit Edge", edgeCb);
    
    pickLabel = new QLabel("Departure: click a server edge in the cluster viewer", this);
    pickLabel->setWordWrap(true);
    navLayout->addRow(pickLabel);
    
    arrivalLabel = new QLabel("Arrival: (none)", this);
    arrivalLabel->setWordWrap(true);
    navLayout->addRow(arrivalLabel);
    
    outer->addWidget(navBox);

    // Cluster Plans
    auto* planBox = new QGroupBox("Cluster Plans", this);
    auto* planLayout = new QGridLayout(planBox);
    planLayout->setContentsMargins(10, 12, 10, 10);
    planLayout->setSpacing(6);
    
    planLayout->addWidget(new QLabel("Plan", this), 0, 0);
    planCb = new QComboBox(this);
    planCb->setEditable(true);
    planLayout->addWidget(planCb, 0, 1, 1, 2);
    
    auto* newPlanBtn = new QPushButton("New", this);
    auto* deletePlanBtn = new QPushButton("Delete Plan", this);
    planLayout->addWidget(newPlanBtn, 1, 1);
    planLayout->addWidget(deletePlanBtn, 1, 2);
    
    planLayout->addWidget(new QLabel("Layer Filter", this), 2, 0);
    layerFilterCb = new QComboBox(this);
    layerFilterCb->addItem("All Layers");
    planLayout->addWidget(layerFilterCb, 2, 1, 1, 2);
    
    outer->addWidget(planBox);

    // Cluster Drawing Tools
    auto* toolsBox = new QGroupBox("Cluster Drawing Tools", this);
    auto* toolsLayout = new QVBoxLayout(toolsBox);
    toolsLayout->setSpacing(8);
    toolsLayout->setContentsMargins(10, 12, 10, 10);

    auto* grid = new QGridLayout();
    grid->setSpacing(6);
    grid->setAlignment(Qt::AlignCenter);

    toolGroup = new QButtonGroup(this);
    toolGroup->setExclusive(true);

    struct ToolInfo {
        QString name;
        QString icon;
        QString value;
    };
    
    std::vector<ToolInfo> tools = {
        {"Pan", "pan_tool", "pan"},
        {"Polyline", "straighten", "polyline"},
        {"Rectangle", "crop_square", "rectangle"},
        {"Circle", "radio_button_unchecked", "circle"},
        {"Arrow", "north_east", "arrow"},
        {"Text", "title", "text"}
    };

    int row = 0, col = 0;
    for (const auto& tool : tools) {
        auto* btn = new QPushButton(tool.icon, this);
        QFont mdi("Material Icons");
        mdi.setPixelSize(22);
        btn->setFont(mdi);
        
        btn->setCheckable(true);
        btn->setFixedSize(48, 48);
        btn->setToolTip(tool.name);
        btn->setProperty("tool_value", tool.value);
        
        btn->setStyleSheet(QString(R"(
            QPushButton {
                background-color: %1;
                border: 1px solid %2;
                border-radius: 8px;
                color: %3;
                font-size: 20px;
            }
            QPushButton:hover {
                background-color: %4;
                border: 1px solid %5;
            }
            QPushButton:checked {
                background-color: %6;
                color: %7;
                border: 1px solid %8;
            }
        )").arg(theme::SURFACE_CARD, theme::BORDER_MUTED, theme::TEXT_PRIMARY, theme::SURFACE_HOVER, theme::ACCENT_EMERALD, theme::ACCENT_TINT, theme::ACCENT_MINT, theme::ACCENT_EMERALD));
        
        toolGroup->addButton(btn);
        grid->addWidget(btn, row, col);
        
        if (tool.value == "pan") btn->setChecked(true);
        
        col++;
        if (col >= 3) {
            col = 0;
            row++;
        }
    }

    toolsLayout->addLayout(grid);

    auto* note = new QLabel("These are separate cluster annotations.", this);
    note->setWordWrap(true);
    note->setAlignment(Qt::AlignCenter);
    note->setStyleSheet(QString("color: %1; font-size: 11px;").arg(theme::TEXT_SECONDARY));
    toolsLayout->addWidget(note);

    outer->addWidget(toolsBox);

    // Objects list
    auto* browserBox = new QGroupBox("Cluster Objects", this);
    auto* browserLayout = new QVBoxLayout(browserBox);
    objectList = new QListWidget(this);
    browserLayout->addWidget(objectList, 1);
    countLabel = new QLabel("0 shown", this);
    browserLayout->addWidget(countLabel);
    outer->addWidget(browserBox, 1);

    // Editor
    auto* editorBox = new QGroupBox("Editor", this);
    auto* editorLayout = new QFormLayout(editorBox);
    
    selectionLabel = new QLabel("New cluster drawing", this);
    selectionLabel->setStyleSheet("font-weight: 700;");
    editorLayout->addRow(selectionLabel);
    
    posLabel = new QLabel("Position: (not selected)", this);
    editorLayout->addRow(posLabel);
    
    objectNameEdit = new QLineEdit("Cluster Drawing", this);
    editorLayout->addRow("Object Name", objectNameEdit);
    
    layerEdit = new QLineEdit("Default", this);
    editorLayout->addRow("Layer", layerEdit);
    
    colorBtn = new QPushButton("#04b97f", this);
    colorBtn->setStyleSheet("background: #04b97f; color: #121214; border: 1px solid #383a42; border-radius: 6px; padding: 6px 10px; font-weight: bold;");
    editorLayout->addRow("Color", colorBtn);
    
    widthSpin = new QSpinBox(this);
    widthSpin->setRange(1, 20);
    widthSpin->setValue(3);
    editorLayout->addRow("Width", widthSpin);
    
    textEdit = new QTextEdit(this);
    textEdit->setMinimumHeight(70);
    textEdit->setPlaceholderText("Label / note");
    editorLayout->addRow("Text / Label", textEdit);
    
    auto* btnRow = new QWidget(this);
    auto* btnRowLayout = new QGridLayout(btnRow);
    btnRowLayout->setContentsMargins(0,0,0,0);
    
    saveBtn = new QPushButton("Save Item", this);
    saveBtn->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; border-radius: 6px; padding: 6px;").arg(theme::ACCENT_EMERALD, theme::TEXT_ON_ACCENT));
    
    newBtn = new QPushButton("New Object", this);
    newBtn->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3; border-radius: 6px; padding: 6px;").arg(theme::SURFACE_CARD, theme::TEXT_PRIMARY, theme::BORDER_MUTED));
    
    removeLastItemBtn = new QPushButton("Remove Last Item", this);
    removeLastItemBtn->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3; border-radius: 6px; padding: 6px;").arg(theme::SURFACE_CARD, theme::TEXT_PRIMARY, theme::BORDER_MUTED));
    
    deleteBtn = new QPushButton("Delete Object", this);
    deleteBtn->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; border-radius: 6px; padding: 6px;").arg(theme::STATUS_DANGER, theme::TEXT_PRIMARY));
    
    btnRowLayout->addWidget(saveBtn, 0, 0);
    btnRowLayout->addWidget(newBtn, 0, 1);
    btnRowLayout->addWidget(removeLastItemBtn, 1, 0);
    btnRowLayout->addWidget(deleteBtn, 1, 1);
    
    editorLayout->addRow(btnRow);
    
    helperLabel = new QLabel("Pan-click a server edge to set departure.", this);
    helperLabel->setWordWrap(true);
    editorLayout->addRow(helperLabel);
    
    outer->addWidget(editorBox);

    // Connections
    connect(modeCb, &QComboBox::currentTextChanged, this, &SailingPanel::recomputeCrossing);
    connect(sourceServerCb, &QComboBox::currentTextChanged, this, &SailingPanel::recomputeCrossing);
    connect(destServerCb, &QComboBox::currentTextChanged, this, &SailingPanel::recomputeCrossing);
    connect(edgeCb, &QComboBox::currentTextChanged, this, &SailingPanel::recomputeCrossing);
    
    connect(planCb, &QComboBox::currentTextChanged, this, &SailingPanel::onPlanChanged);
    connect(layerFilterCb, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshObjectList();
        emit dataChanged();
    });
    
    connect(newPlanBtn, &QPushButton::clicked, this, &SailingPanel::newPlan);
    connect(deletePlanBtn, &QPushButton::clicked, this, &SailingPanel::deletePlan);
    
    connect(toolGroup, &QButtonGroup::buttonToggled, this, [this](QAbstractButton*, bool checked) {
        if (checked) onToolChanged();
    });
    
    connect(colorBtn, &QPushButton::clicked, this, &SailingPanel::pickColor);
    connect(objectList, &QListWidget::currentRowChanged, this, &SailingPanel::onObjectSelected);
    
    connect(saveBtn, &QPushButton::clicked, this, &SailingPanel::saveCurrent);
    connect(newBtn, &QPushButton::clicked, this, &SailingPanel::resetEditor);
    connect(removeLastItemBtn, &QPushButton::clicked, this, &SailingPanel::removeLastItem);
    connect(deleteBtn, &QPushButton::clicked, this, &SailingPanel::deleteCurrent);
}

// Methods setContext, getDisplayImage, getOverlays, buildOverlays, and recomputeCrossing
// are defined in SailingPanelOverlays.cpp to keep file size under 500 lines.

void SailingPanel::refreshPlanNames() {
    QString current = planCb->currentText().trimmed();
    if (current.isEmpty()) current = "Default Plan";
    
    auto names = drawingStore->getPlanNames(layoutModel.name);
    planCb->blockSignals(true);
    planCb->clear();
    for (const auto& n : names) planCb->addItem(QString::fromStdString(n));
    planCb->setEditText(current);
    planCb->blockSignals(false);
}

void SailingPanel::refreshLayerFilter() {
    QString current = layerFilterCb->currentText().trimmed();
    if (current.isEmpty()) current = "All Layers";
    
    auto objects = drawingStore->getByPlan(layoutModel.name, planCb->currentText().trimmed().toStdString());
    std::vector<std::string> layers;
    for (const auto& obj : objects) {
        std::string layer = obj.layer.empty() ? "Default" : obj.layer;
        if (std::find(layers.begin(), layers.end(), layer) == layers.end()) {
            layers.push_back(layer);
        }
    }
    std::sort(layers.begin(), layers.end());
    
    layerFilterCb->blockSignals(true);
    layerFilterCb->clear();
    layerFilterCb->addItem("All Layers");
    for (const auto& l : layers) layerFilterCb->addItem(QString::fromStdString(l));
    
    int idx = layerFilterCb->findText(current);
    layerFilterCb->setCurrentIndex(idx >= 0 ? idx : 0);
    layerFilterCb->blockSignals(false);
}

void SailingPanel::refreshObjectList() {
    objectList->blockSignals(true);
    objectList->clear();
    filteredObjects.clear();
    
    QString layerFilter = layerFilterCb->currentText().trimmed();
    auto objects = drawingStore->getByPlan(layoutModel.name, planCb->currentText().trimmed().toStdString());
    
    int selectedRow = -1;
    for (const auto& obj : objects) {
        if (layerFilter != "All Layers" && QString::fromStdString(obj.layer) != layerFilter) continue;
        
        int row = filteredObjects.size();
        filteredObjects.push_back(obj);
        objectList->addItem(QString("[%1] %2 (%3 items)").arg(QString::fromStdString(obj.layer), QString::fromStdString(obj.name)).arg(obj.items.size()));
        
        if (obj.id == currentObjectId) selectedRow = row;
    }
    
    countLabel->setText(QString("%1 shown").arg(filteredObjects.size()));
    if (selectedRow >= 0) objectList->setCurrentRow(selectedRow);
    
    objectList->blockSignals(false);
}

void SailingPanel::onPlanChanged() {
    currentObjectId = "";
    pendingPoints.clear();
    refreshLayerFilter();
    refreshObjectList();
    emit dataChanged();
}

void SailingPanel::onToolChanged() {
    pendingPoints.clear();
    auto btn = toolGroup->checkedButton();
    QString tool = btn ? btn->property("tool_value").toString() : "pan";
    
    if (tool == "pan") helperLabel->setText("Pan mode. Click a server edge to set departure.");
    else if (tool == "polyline") helperLabel->setText("Click to add cluster polyline points.");
    else if (tool == "rectangle" || tool == "circle" || tool == "arrow") helperLabel->setText(QString("Click-drag to preview a %1.").arg(tool));
    else if (tool == "text") helperLabel->setText("Click once to place a cluster text note.");
    
    emit dataChanged();
}

void SailingPanel::pickColor() {
    QColor color = QColorDialog::getColor(QColor(colorBtn->text()), this);
    if (color.isValid()) {
        colorBtn->setText(color.name());
        colorBtn->setStyleSheet(QString("background: %1; color: #ffffff; border: 1px solid #444444; border-radius: 6px; padding: 6px 10px;").arg(color.name()));
        emit dataChanged();
    }
}

void SailingPanel::newPlan() {
    planCb->setEditText("New Plan");
    currentObjectId = "";
    pendingPoints.clear();
    selectionLabel->setText("New cluster drawing");
    objectList->clearSelection();
    helperLabel->setText("Type a new cluster plan name and start drawing.");
    refreshObjectList();
    emit dataChanged();
}

void SailingPanel::resetEditor() {
    currentObjectId = "";
    pendingPoints.clear();
    objectNameEdit->setText("Cluster Drawing");
    layerEdit->setText("Default");
    widthSpin->setValue(3);
    textEdit->clear();
    selectionLabel->setText("New cluster drawing");
    posLabel->setText("Position: (not selected)");
    objectList->clearSelection();
    helperLabel->setText("Starting a new cluster drawing object.");
    emit dataChanged();
}

void SailingPanel::onObjectSelected(int currentRow) {
    if (currentRow < 0 || currentRow >= (int)filteredObjects.size()) return;
    const auto& obj = filteredObjects[currentRow];
    currentObjectId = obj.id;
    pendingPoints.clear();
    
    objectNameEdit->setText(QString::fromStdString(obj.name));
    layerEdit->setText(QString::fromStdString(obj.layer));
    if (!obj.items.empty()) {
        const auto& last = obj.items.back();
        colorBtn->setText(QString::fromStdString(last.color));
        colorBtn->setStyleSheet(QString("background: %1; color: #ffffff; border: 1px solid #444444; border-radius: 6px; padding: 6px 10px;").arg(QString::fromStdString(last.color)));
        widthSpin->setValue(last.width);
        textEdit->setPlainText(QString::fromStdString(last.label));
    }
    
    selectionLabel->setText(QString("Editing cluster object: %1").arg(QString::fromStdString(obj.name)));
    posLabel->setText(QString("Items: %1").arg(obj.items.size()));
    helperLabel->setText("Cluster object selected. Save appends more items to it.");
    emit dataChanged();
}

// CRUD and Drawing store action methods (deleteCurrent, removeLastItem, deletePlan, saveCurrent)
// along with overlay drawing and context resolution are defined in SailingPanelOverlays.cpp
// to keep file size well under the 500 lines limit.

} // namespace ui
} // namespace treasure
