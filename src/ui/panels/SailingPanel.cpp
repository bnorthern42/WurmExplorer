#include "SailingPanel.hpp"

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
#include <QRadioButton>
#include <QButtonGroup>
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
    
    planLayout->addWidget(new QLabel("Plan", this), 0, 0);
    planCb = new QComboBox(this);
    planCb->setEditable(true);
    planLayout->addWidget(planCb, 0, 1);
    
    auto* newPlanBtn = new QPushButton("New", this);
    auto* deletePlanBtn = new QPushButton("Delete Plan", this);
    planLayout->addWidget(newPlanBtn, 0, 2);
    planLayout->addWidget(deletePlanBtn, 0, 3);
    
    planLayout->addWidget(new QLabel("Layer Filter", this), 1, 0);
    layerFilterCb = new QComboBox(this);
    layerFilterCb->addItem("All Layers");
    planLayout->addWidget(layerFilterCb, 1, 1, 1, 3);
    
    outer->addWidget(planBox);

    // Cluster Drawing Tools
    auto* toolsBox = new QGroupBox("Cluster Drawing Tools", this);
    auto* toolsLayout = new QGridLayout(toolsBox);
    toolGroup = new QButtonGroup(this);
    
    auto addTool = [&](const QString& text, const QString& value, int row, int col, bool checked) {
        auto* btn = new QRadioButton(text, this);
        btn->setProperty("tool_value", value);
        btn->setChecked(checked);
        toolGroup->addButton(btn);
        toolsLayout->addWidget(btn, row, col);
    };
    
    addTool("Pan", "pan", 0, 0, true);
    addTool("Polyline", "polyline", 0, 1, false);
    addTool("Rectangle", "rectangle", 0, 2, false);
    addTool("Circle", "circle", 1, 0, false);
    addTool("Arrow", "arrow", 1, 1, false);
    addTool("Text", "text", 1, 2, false);
    
    auto* note = new QLabel("These are separate cluster annotations.", this);
    note->setWordWrap(true);
    note->setStyleSheet("color: gray;");
    toolsLayout->addWidget(note, 2, 0, 1, 3);
    
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
    
    colorBtn = new QPushButton("#66d9ff", this);
    colorBtn->setStyleSheet("background: #66d9ff; color: #ffffff; border: 1px solid #444444; border-radius: 6px; padding: 6px 10px;");
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
    newBtn = new QPushButton("New Object", this);
    removeLastItemBtn = new QPushButton("Remove Last Item", this);
    deleteBtn = new QPushButton("Delete Object", this);
    
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

void SailingPanel::setContext(const std::map<std::string, std::shared_ptr<treasure::core::ServerConfig>>& allConfigs, const std::string& currentMapType, const std::map<std::string, std::string>& mapImages) {
    cfgByName = allConfigs;
    this->currentMapType = currentMapType;
    if (this->currentMapType != "terrain" && this->currentMapType != "topo" && this->currentMapType != "classic") {
        this->currentMapType = "terrain";
    }
    this->currentMapImages = mapImages;
    
    std::map<std::string, int> serverSizes;
    for (const auto& [name, cfg] : allConfigs) {
        serverSizes[name] = cfg->map_size_tiles;
    }
    
    layoutModel = treasure::models::SailingLogic::buildLayout(serverSizes);
    
    // Setup server dropdowns
    std::vector<std::string> serverOrder = {
        "Independence", "Deliverance", "Exodus", "Celebration", 
        "Pristine", "Release", "Xanadu", "Chaos"
    };
    
    QString currentSource = sourceServerCb->currentText();
    QString currentDest = destServerCb->currentText();
    
    sourceServerCb->blockSignals(true);
    destServerCb->blockSignals(true);
    sourceServerCb->clear();
    destServerCb->clear();
    
    for (const auto& name : serverOrder) {
        if (layoutModel.servers.find(name) != layoutModel.servers.end()) {
            sourceServerCb->addItem(QString::fromStdString(name));
            destServerCb->addItem(QString::fromStdString(name));
        }
    }
    
    int sIdx = sourceServerCb->findText(currentSource);
    if (sIdx >= 0) sourceServerCb->setCurrentIndex(sIdx);
    
    int dIdx = destServerCb->findText(currentDest);
    if (dIdx >= 0) destServerCb->setCurrentIndex(dIdx);
    
    sourceServerCb->blockSignals(false);
    destServerCb->blockSignals(false);
    
    // Force rerender image
    renderState = treasure::models::SailingLogic::renderClusterMap(layoutModel, currentMapImages, this->currentMapType);
    
    refreshPlanNames();
    refreshLayerFilter();
    refreshObjectList();
    recomputeCrossing();
    
    emit imageChanged();
    emit dataChanged();
}

QImage SailingPanel::getDisplayImage() const {
    if (renderState) return renderState->image;
    return QImage();
}

treasure::ui::CanvasOverlays SailingPanel::getOverlays() const {
    return cachedOverlays;
}

void SailingPanel::buildOverlays() {
    cachedOverlays.clear();
    if (!renderState) return;
    
    if (currentResult) {
        auto srcPx = treasure::models::SailingLogic::globalToImagePx(currentResult->source_global_tile_x, currentResult->source_global_tile_y, *renderState);
        auto dstPx = treasure::models::SailingLogic::globalToImagePx(currentResult->dest_global_tile_x, currentResult->dest_global_tile_y, *renderState);
        
        QColor color = currentResult->mode == "regular" ? QColor(0, 255, 255) : QColor(255, 210, 76);
        
        OverlayLine line;
        line.points.push_back(srcPx);
        line.points.push_back(dstPx);
        line.color = color;
        line.width = 4;
        line.dashPattern = {6.0, 4.0};
        cachedOverlays.lines.push_back(line);
        
        cachedOverlays.points.push_back(OverlayPoint{srcPx.first, srcPx.second, color});
        cachedOverlays.points.push_back(OverlayPoint{dstPx.first, dstPx.second, QColor(255, 210, 76)});
        
        cachedOverlays.texts.push_back(OverlayText{srcPx.first, srcPx.second, QString("Depart %1").arg(QString::fromStdString(currentResult->source_server)), color});
        cachedOverlays.texts.push_back(OverlayText{dstPx.first, dstPx.second, QString("Arrive %1").arg(QString::fromStdString(currentResult->dest_server)), QColor(255, 210, 76)});
    }
    
    QString layerFilter = layerFilterCb->currentText().trimmed();
    auto objects = drawingStore->getByPlan(layoutModel.name, planCb->currentText().trimmed().toStdString());
    
    auto globalToPxItem = [&](const std::vector<treasure::models::Point>& pts) {
        std::vector<std::pair<float, float>> pxPts;
        for (const auto& p : pts) {
            pxPts.push_back(treasure::models::SailingLogic::globalToImagePx(p.x, p.y, *renderState));
        }
        return pxPts;
    };
    auto globalToPxPending = [&](const std::vector<std::pair<float, float>>& pts) {
        std::vector<std::pair<float, float>> pxPts;
        for (const auto& p : pts) {
            pxPts.push_back(treasure::models::SailingLogic::globalToImagePx(p.first, p.second, *renderState));
        }
        return pxPts;
    };
    
    for (const auto& obj : objects) {
        if (layerFilter != "All Layers" && QString::fromStdString(obj.layer) != layerFilter) continue;
        for (const auto& sub : obj.items) {
            QColor color(QString::fromStdString(sub.color.empty() ? "#66d9ff" : sub.color));
            int width = std::max(1, sub.width);
            auto ptsPx = globalToPxItem(sub.points);
            
            if (sub.tool == "polyline" && ptsPx.size() >= 2) {
                cachedOverlays.lines.push_back(OverlayLine{ptsPx, color, width, {}});
            } else if (sub.tool == "rectangle" && ptsPx.size() == 2) {
                OverlayPolygon poly;
                float x1 = ptsPx[0].first, y1 = ptsPx[0].second;
                float x2 = ptsPx[1].first, y2 = ptsPx[1].second;
                poly.points = {std::make_pair(x1, y1), std::make_pair(x2, y1), std::make_pair(x2, y2), std::make_pair(x1, y2)};
                poly.lineColor = color;
                poly.fillColor = QColor();
                poly.width = width;
                cachedOverlays.polygons.push_back(poly);
            } else if (sub.tool == "circle" && ptsPx.size() == 2) {
                float dx = ptsPx[1].first - ptsPx[0].first;
                float dy = ptsPx[1].second - ptsPx[0].second;
                float r = std::hypot(dx, dy);
                cachedOverlays.circles.push_back(OverlayCircle{ptsPx[0].first, ptsPx[0].second, r, color});
            } else if (sub.tool == "arrow" && ptsPx.size() == 2) {
                // simple arrow line for now
                cachedOverlays.lines.push_back(OverlayLine{ptsPx, color, width, {}});
            } else if (sub.tool == "text" && ptsPx.size() == 1) {
                cachedOverlays.points.push_back(OverlayPoint{ptsPx[0].first, ptsPx[0].second, color});
                cachedOverlays.texts.push_back(OverlayText{ptsPx[0].first, ptsPx[0].second, QString::fromStdString(sub.label.empty() ? "(text)" : sub.label), color});
            }
            
            if (!sub.label.empty() && sub.tool != "text" && !ptsPx.empty()) {
                auto mp = ptsPx[ptsPx.size() / 2];
                cachedOverlays.texts.push_back(OverlayText{mp.first, mp.second, QString::fromStdString(sub.label), color});
            }
        }
    }
    
    // Preview
    QColor previewColor("#ff66ff");
    int previewWidth = std::max(1, widthSpin->value());
    QString previewTool = toolGroup->checkedButton() ? toolGroup->checkedButton()->property("tool_value").toString() : "pan";
    auto previewPtsPx = globalToPxPending(pendingPoints);
    
    if (previewTool == "polyline" && previewPtsPx.size() >= 2) {
        cachedOverlays.lines.push_back(OverlayLine{previewPtsPx, previewColor, previewWidth, {4.0, 4.0}});
    } else if (previewTool == "rectangle" && previewPtsPx.size() == 2) {
        OverlayPolygon poly;
        float x1 = previewPtsPx[0].first, y1 = previewPtsPx[0].second;
        float x2 = previewPtsPx[1].first, y2 = previewPtsPx[1].second;
        poly.points = {std::make_pair(x1, y1), std::make_pair(x2, y1), std::make_pair(x2, y2), std::make_pair(x1, y2)};
        poly.lineColor = previewColor;
        poly.fillColor = QColor();
        poly.width = previewWidth;
        cachedOverlays.polygons.push_back(poly);
    } else if (previewTool == "circle" && previewPtsPx.size() == 2) {
        float dx = previewPtsPx[1].first - previewPtsPx[0].first;
        float dy = previewPtsPx[1].second - previewPtsPx[0].second;
        float r = std::hypot(dx, dy);
        cachedOverlays.circles.push_back(OverlayCircle{previewPtsPx[0].first, previewPtsPx[0].second, r, previewColor});
    } else if (previewTool == "arrow" && previewPtsPx.size() == 2) {
        cachedOverlays.lines.push_back(OverlayLine{previewPtsPx, previewColor, previewWidth, {4.0, 4.0}});
    } else if (previewTool == "text" && previewPtsPx.size() == 1) {
        QString txt = textEdit->toPlainText().trimmed();
        cachedOverlays.points.push_back(OverlayPoint{previewPtsPx[0].first, previewPtsPx[0].second, previewColor});
        cachedOverlays.texts.push_back(OverlayText{previewPtsPx[0].first, previewPtsPx[0].second, txt.isEmpty() ? "(text)" : txt, previewColor});
    }
    
    for (const auto& p : previewPtsPx) {
        cachedOverlays.points.push_back(OverlayPoint{p.first, p.second, previewColor});
    }
}

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

void SailingPanel::deleteCurrent() {
    if (!currentObjectId.empty()) {
        drawingStore->remove(currentObjectId);
        currentObjectId = "";
        pendingPoints.clear();
        refreshObjectList();
        refreshLayerFilter();
        helperLabel->setText("Cluster object deleted.");
        emit dataChanged();
    }
}

void SailingPanel::removeLastItem() {
    if (currentObjectId.empty()) {
        helperLabel->setText("Select a cluster object first.");
        return;
    }
    auto objOpt = drawingStore->get(currentObjectId);
    if (!objOpt || objOpt->items.empty()) {
        helperLabel->setText("Object has no items.");
        return;
    }
    auto obj = *objOpt;
    obj.items.pop_back();
    if (!obj.items.empty()) {
        drawingStore->update(obj);
        helperLabel->setText("Removed last cluster item.");
    } else {
        drawingStore->remove(obj.id);
        currentObjectId = "";
        helperLabel->setText("Removed last item. Cluster object was deleted.");
    }
    pendingPoints.clear();
    refreshObjectList();
    refreshLayerFilter();
    emit dataChanged();
}

void SailingPanel::deletePlan() {
    QString planName = planCb->currentText().trimmed();
    drawingStore->removePlan(layoutModel.name, planName.toStdString());
    currentObjectId = "";
    pendingPoints.clear();
    refreshPlanNames();
    refreshLayerFilter();
    refreshObjectList();
    helperLabel->setText(QString("Deleted cluster plan: %1").arg(planName));
    emit dataChanged();
}

void SailingPanel::saveCurrent() {
    auto btn = toolGroup->checkedButton();
    QString tool = btn ? btn->property("tool_value").toString() : "pan";
    QString label = textEdit->toPlainText().trimmed();
    
    if (tool == "polyline" && pendingPoints.size() < 2) {
        helperLabel->setText("Polyline needs at least 2 points.");
        return;
    }
    if ((tool == "rectangle" || tool == "circle" || tool == "arrow") && pendingPoints.size() != 2) {
        helperLabel->setText(QString("%1 needs exactly 2 points.").arg(tool));
        return;
    }
    if (tool == "text") {
        if (pendingPoints.size() != 1) {
            helperLabel->setText("Text needs exactly 1 point.");
            return;
        }
        if (label.isEmpty()) {
            helperLabel->setText("Enter text before saving.");
            return;
        }
    }
    
    treasure::models::ClusterDrawingItem item;
    item.tool = tool.toStdString();
    item.color = colorBtn->text().toStdString();
    item.width = widthSpin->value();
    item.label = label.toStdString();
    for (const auto& p : pendingPoints) {
        item.points.push_back({p.first, p.second});
    }
    
    QString objName = objectNameEdit->text().trimmed();
    if (objName.isEmpty()) objName = "Cluster Drawing";
    QString layer = layerEdit->text().trimmed();
    if (layer.isEmpty()) layer = "Default";
    QString planName = planCb->currentText().trimmed();
    if (planName.isEmpty()) planName = "Default Plan";
    
    if (!currentObjectId.empty()) {
        auto objOpt = drawingStore->get(currentObjectId);
        if (objOpt) {
            auto obj = *objOpt;
            obj.plan_name = planName.toStdString();
            obj.layer = layer.toStdString();
            obj.name = objName.toStdString();
            obj.items.push_back(item);
            drawingStore->update(obj);
            helperLabel->setText(QString("Added new %1 item to '%2'.").arg(tool, QString::fromStdString(obj.name)));
        } else {
            helperLabel->setText("Selected object no longer exists.");
            return;
        }
    } else {
        treasure::models::ClusterDrawingObject obj;
        obj.id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        obj.cluster = layoutModel.name;
        obj.plan_name = planName.toStdString();
        obj.layer = layer.toStdString();
        obj.name = objName.toStdString();
        obj.items.push_back(item);
        drawingStore->add(obj);
        currentObjectId = obj.id;
        helperLabel->setText(QString("Created cluster object '%1'.").arg(QString::fromStdString(obj.name)));
    }
    
    pendingPoints.clear();
    selectionLabel->setText(QString("Editing cluster object: %1").arg(objName));
    posLabel->setText("Position: (not selected)");
    refreshPlanNames();
    refreshLayerFilter();
    refreshObjectList();
    emit dataChanged();
}

void SailingPanel::recomputeCrossing() {
    QString mode = modeCb->currentText();
    QString sourceServer = sourceServerCb->currentText();
    QString destServer = destServerCb->currentText();
    QString edge = edgeCb->currentText();
    
    if (mode == "plot_course") {
        currentResult = treasure::models::SailingLogic::resolvePlotCourse(layoutModel, sourceServer.toStdString(), edge.toStdString(), 0.5f, destServer.toStdString());
    } else {
        currentResult = treasure::models::SailingLogic::resolveRegularCrossing(layoutModel, sourceServer.toStdString(), edge.toStdString(), 0.5f);
    }
    
    if (currentResult) {
        pickLabel->setText(QString("Departure: %1 on %2 edge").arg(QString::fromStdString(currentResult->source_server), QString::fromStdString(currentResult->source_edge)));
        arrivalLabel->setText(QString("Arrival: %1 on %2 edge").arg(QString::fromStdString(currentResult->dest_server), QString::fromStdString(currentResult->dest_edge)));
    } else {
        pickLabel->setText("Departure: (Invalid route)");
        arrivalLabel->setText("Arrival: (none)");
    }
    
    buildOverlays();
    emit dataChanged();
}

} // namespace ui
} // namespace treasure
