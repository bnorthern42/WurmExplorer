#pragma once

#include <QWidget>
#include <QListWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QColorDialog>
#include <memory>
#include <string>

#include "../../models/DrawingStore.hpp"
#include "../../core/Config.hpp"

class DrawingPanel : public QWidget {
    Q_OBJECT

public:
    explicit DrawingPanel(std::shared_ptr<treasure::models::DrawingStore> store, QWidget *parent = nullptr);
    ~DrawingPanel() override = default;

    void setContext(std::shared_ptr<treasure::core::ServerConfig> cfg, const QString& layer);
    void refreshList();
    void undoLast();
    
    int getLineWidth() const;
    QString getCurrentColor() const;

signals:
    void toolSelected(const QString& toolName);
    void propertiesChanged(); // width or color changed
    void requestRedraw();     // need to repaint canvas

private slots:
    void onLineWidthChanged(int val);
    void onDeleteSelected();
    void onClearAll();

private:
    void setupUi();

    std::shared_ptr<treasure::models::DrawingStore> drawingStore;
    std::shared_ptr<treasure::core::ServerConfig> currentConfig;
    QString currentLayer;
    
    QButtonGroup* toolGroup;
    QColorDialog* colorPicker;
    QSpinBox* widthSpin;
    QListWidget* objectList;
    
    std::vector<treasure::models::DrawingObject> visibleObjects;
};
