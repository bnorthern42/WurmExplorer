#include "DrawingPanel.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QMessageBox>
#include <QScrollArea>
#include <QColorDialog>

DrawingPanel::DrawingPanel(std::shared_ptr<treasure::models::DrawingStore> store, QWidget *parent) 
    : QWidget(parent), drawingStore(store) {
    setupUi();
}

void DrawingPanel::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(12);

    auto* titleLabel = new QLabel("Drawing Tools", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #8aadf4;");
    layout->addWidget(titleLabel);

    // --- 1. Tool Palette ---
    auto* grid = new QGridLayout();
    grid->setSpacing(4);

    toolGroup = new QButtonGroup(this);
    toolGroup->setExclusive(true);

    struct ToolInfo {
        QString name;
        QString icon;
    };
    
    std::vector<ToolInfo> tools = {
        {"Drag", "pan_tool"},
        {"Line", "straighten"},
        {"Freehand", "gesture"},
        {"Highlighter", "highlight"},
        {"Rectangle", "crop_square"},
        {"Circle", "radio_button_unchecked"},
        {"Marker", "place"},
        {"Text", "title"},
        {"Measure", "square_foot"},
        {"Eraser", "backspace"}
    };
    
    int row = 0, col = 0;
    for (const auto& tool : tools) {
        auto* btn = new QPushButton(tool.icon, this);
        QFont mdi("Material Icons");
        mdi.setPixelSize(24);
        btn->setFont(mdi);
        
        btn->setCheckable(true);
        btn->setFixedSize(48, 48);
        btn->setToolTip(tool.name);
        btn->setProperty("toolName", tool.name);
        
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: #313244;
                border-radius: 8px;
                color: #cdd6f4;
                font-size: 20px;
            }
            QPushButton:hover {
                background-color: #45475a;
            }
            QPushButton:checked {
                background-color: #8aadf4;
                color: #11111b;
            }
        )");
        
        toolGroup->addButton(btn);
        grid->addWidget(btn, row, col);
        
        if (tool.name == "Drag") btn->setChecked(true);
        
        col++;
        if (col >= 3) {
            col = 0;
            row++;
        }
    }

    connect(toolGroup, &QButtonGroup::buttonToggled, this, [this](QAbstractButton* btn, bool checked) {
        if (checked) {
            emit toolSelected(btn->property("toolName").toString());
        }
    });

    layout->addLayout(grid);
    layout->addSpacing(8);

    auto* propsGroup = new QGroupBox("Properties", this);
    auto* propsLayout = new QVBoxLayout(propsGroup);
    
    auto* widthLayout = new QHBoxLayout();
    widthLayout->addWidget(new QLabel("Line Width:"));
    widthSpin = new QSpinBox(this);
    widthSpin->setRange(1, 20);
    widthSpin->setValue(2);
    widthLayout->addWidget(widthSpin);
    propsLayout->addLayout(widthLayout);
    
    colorPicker = new QColorDialog(this);
    colorPicker->setWindowFlags(Qt::Widget);
    colorPicker->setOptions(QColorDialog::NoButtons | QColorDialog::DontUseNativeDialog);
    colorPicker->setCurrentColor(QColor("#ed8796"));
    
    propsLayout->addWidget(colorPicker);
    layout->addWidget(propsGroup);
    
    connect(widthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &DrawingPanel::propertiesChanged);
    connect(colorPicker, &QColorDialog::currentColorChanged, this, [this](const QColor&){
        emit propertiesChanged();
    });

    // --- 3. Objects Manager ---
    auto* objGroup = new QGroupBox("Objects", this);
    auto* objLayout = new QVBoxLayout(objGroup);
    
    objectList = new QListWidget(this);
    objLayout->addWidget(objectList);
    
    auto* btnRow = new QHBoxLayout();
    auto* delBtn = new QPushButton("Delete Selected", this);
    auto* clearBtn = new QPushButton("Clear All", this);
    
    delBtn->setStyleSheet("background-color: #ed8796; color: #11111b; font-weight: bold;");
    clearBtn->setStyleSheet("background-color: #45475a; color: #cdd6f4;");
    
    btnRow->addWidget(delBtn);
    btnRow->addWidget(clearBtn);
    objLayout->addLayout(btnRow);
    
    layout->addWidget(objGroup, 1); // List takes remaining space
    
    connect(delBtn, &QPushButton::clicked, this, &DrawingPanel::onDeleteSelected);
    connect(clearBtn, &QPushButton::clicked, this, &DrawingPanel::onClearAll);
}

void DrawingPanel::setContext(std::shared_ptr<treasure::core::ServerConfig> cfg, const QString& layer) {
    currentConfig = cfg;
    currentLayer = layer;
    refreshList();
}

void DrawingPanel::refreshList() {
    objectList->clear();
    visibleObjects.clear();
    
    if (!drawingStore || !currentConfig) return;
    
    auto allObjects = drawingStore->getByServer(currentConfig->name);
    
    int count = 1;
    for (const auto& obj : allObjects) {
        if (obj.layer == currentLayer.toStdString()) {
            visibleObjects.push_back(obj);
            
            QString label = QString::fromStdString(obj.name);
            if (label == "Shape") {
                if (!obj.items.empty()) {
                    label = QString("%1 %2").arg(QString::fromStdString(obj.items[0].color)).arg(QString::fromStdString(obj.items[0].tool));
                }
            }
            objectList->addItem(label + " #" + QString::number(count++));
        }
    }
}

void DrawingPanel::onLineWidthChanged(int /*val*/) {
    emit propertiesChanged();
}

int DrawingPanel::getLineWidth() const {
    return widthSpin->value();
}

QString DrawingPanel::getCurrentColor() const {
    return colorPicker->currentColor().name();
}

void DrawingPanel::undoLast() {
    if (visibleObjects.empty()) return;
    
    // The last item added is at the end of the list
    drawingStore->remove(visibleObjects.back().id);
    drawingStore->save();
    refreshList();
    emit requestRedraw();
}

void DrawingPanel::onDeleteSelected() {
    int idx = objectList->currentRow();
    if (idx >= 0 && idx < static_cast<int>(visibleObjects.size())) {
        drawingStore->remove(visibleObjects[idx].id);
        drawingStore->save();
        refreshList();
        emit requestRedraw();
    }
}

void DrawingPanel::onClearAll() {
    if (visibleObjects.empty()) return;
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Clear All", "Are you sure you want to delete all drawings on this layer?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        for (const auto& obj : visibleObjects) {
            drawingStore->remove(obj.id);
        }
        drawingStore->save();
        refreshList();
        emit requestRedraw();
    }
}
