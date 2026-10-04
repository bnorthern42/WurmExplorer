#include "SailingPanel.hpp"
#include "../../models/ClusterLayout.hpp"

#include <QColor>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QButtonGroup>
#include <QAbstractButton>
#include <QUuid>
#include <cmath>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

namespace treasure {
namespace ui {

void SailingPanel::setContext(const std::map<std::string, std::shared_ptr<treasure::core::ServerConfig>>& allConfigs,
                             const std::string& currentMapType,
                             const std::map<std::string, std::string>& mapImages,
                             const std::string& clusterName) {
    (void)clusterName;
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
    
    layoutModel = treasure::models::SailingLogic::buildLayout(serverSizes, clusterName);
    
    // Setup server dropdowns
    std::vector<std::string> preferredOrder = {
        "Independence", "Deliverance", "Exodus", "Celebration", 
        "Pristine", "Release", "Xanadu", "Chaos",
        "Harmony", "Melody", "Cadence", "Defiance",
        "Elevation", "Desertion", "Serenity", "Affliction"
    };
    
    QString currentSource = sourceServerCb->currentText();
    QString currentDest = destServerCb->currentText();
    
    sourceServerCb->blockSignals(true);
    destServerCb->blockSignals(true);
    sourceServerCb->clear();
    destServerCb->clear();
    
    std::vector<std::string> added;
    for (const auto& name : preferredOrder) {
        if (layoutModel.servers.find(name) != layoutModel.servers.end()) {
            sourceServerCb->addItem(QString::fromStdString(name));
            destServerCb->addItem(QString::fromStdString(name));
            added.push_back(name);
        }
    }
    for (const auto& [name, _] : layoutModel.servers) {
        if (std::find(added.begin(), added.end(), name) == added.end()) {
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
            QColor color(QString::fromStdString(sub.color.empty() ? "#04b97f" : sub.color));
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
        cachedOverlays.points.push_back(OverlayPoint{previewPtsPx[0].first, previewPtsPx[0].second, previewColor});
        cachedOverlays.texts.push_back(OverlayText{previewPtsPx[0].first, previewPtsPx[0].second, QString("(text: %1)").arg(textEdit->toPlainText().trimmed()), previewColor});
    }
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

} // namespace ui
} // namespace treasure
