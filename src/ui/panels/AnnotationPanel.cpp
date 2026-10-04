#include "AnnotationPanel.hpp"
#include "../ThemeTokens.hpp"
#include <QHBoxLayout>
#include <algorithm>
#include <QUuid>
#include <QStringList>

namespace treasure {
namespace ui {

AnnotationPanel::AnnotationPanel(std::shared_ptr<models::AnnotationStore> store, QWidget* parent)
    : QWidget(parent), annotStore(store) {
    
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(10, 10, 10, 10);
    
    createToolGroup();
    createBrowserGroup();
    createEditorGroup();
    
    outer->addWidget(findChild<QGroupBox*>("toolsGroup"));
    outer->addWidget(findChild<QGroupBox*>("browserGroup"), 1);
    outer->addWidget(findChild<QGroupBox*>("editorGroup"));
    
    kingdomRow->hide();
    influenceRow->hide();
    
    refreshList();
}

void AnnotationPanel::createToolGroup() {
    QGroupBox* toolsFrame = new QGroupBox("Drawing Tools", this);
    toolsFrame->setObjectName("toolsGroup");
    QVBoxLayout* layout = new QVBoxLayout(toolsFrame);
    layout->setSpacing(12);
    
    toolGroup = new QButtonGroup(this);
    drawGroup = new QButtonGroup(this);
    
    // Segmented button group using an HBoxLayout
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(4);
    
    panBtn = createToolButton("🖐 Pan", "pan", true);
    deedBtn = createToolButton("🏰 Deed", "add_deed");
    roadBtn = createToolButton("🛣️ Road", "add_road");
    
    btnLayout->addWidget(panBtn);
    btnLayout->addWidget(deedBtn);
    btnLayout->addWidget(roadBtn);
    layout->addLayout(btnLayout);
    
    QHBoxLayout* btnLayout2 = new QHBoxLayout();
    btnLayout2->setSpacing(4);
    bridgeBtn = createToolButton("🌉 Bridge", "add_bridge");
    tunnelBtn = createToolButton("🚇 Tunnel", "add_tunnel");
    guardBtn = createToolButton("🗼 Guard", "add_guard_tower");
    
    btnLayout2->addWidget(bridgeBtn);
    btnLayout2->addWidget(tunnelBtn);
    btnLayout2->addWidget(guardBtn);
    layout->addLayout(btnLayout2);
    
    QHBoxLayout* styleRow = new QHBoxLayout();
    styleRow->addWidget(new QLabel("Draw style:"));
    
    lineDrawBtn = new QRadioButton("Line");
    brushDrawBtn = new QRadioButton("Brush");
    lineDrawBtn->setChecked(true);
    
    drawGroup->addButton(lineDrawBtn);
    drawGroup->addButton(brushDrawBtn);
    
    styleRow->addWidget(lineDrawBtn);
    styleRow->addWidget(brushDrawBtn);
    styleRow->addStretch(1);
    
    layout->addLayout(styleRow);
    
    QLabel* hint = new QLabel("Left click adds points. Right click exits draw mode. Ctrl snaps segments.");
    hint->setWordWrap(true);
    hint->setStyleSheet(QString("color: %1; font-size: 11px;").arg(theme::TEXT_SECONDARY)); // Muted hint
    layout->addWidget(hint);
}

void AnnotationPanel::createBrowserGroup() {
    QGroupBox* browser = new QGroupBox("Annotations", this);
    browser->setObjectName("browserGroup");
    QGridLayout* layout = new QGridLayout(browser);
    
    searchEdit = new QLineEdit();
    filterCb = new QComboBox();
    filterCb->addItems({"All", "Deed", "Road", "Bridge", "Tunnel"});
    
    annotList = new QListWidget();
    
    layout->addWidget(new QLabel("Search"), 0, 0);
    layout->addWidget(searchEdit, 0, 1);
    layout->addWidget(new QLabel("Type"), 1, 0);
    layout->addWidget(filterCb, 1, 1);
    layout->addWidget(annotList, 2, 0, 1, 2);
    
    QHBoxLayout* footer = new QHBoxLayout();
    countLabel = new QLabel("0 shown");
    footer->addWidget(countLabel);
    QLabel* footNote = new QLabel("Manual annotations only. Imported data lives on Map Data tab.");
    footNote->setAlignment(Qt::AlignRight);
    footer->addWidget(footNote, 1);
    
    layout->addLayout(footer, 3, 0, 1, 2);
    
    connect(searchEdit, &QLineEdit::textChanged, this, &AnnotationPanel::onSearchFilterChanged);
    connect(filterCb, &QComboBox::currentIndexChanged, this, &AnnotationPanel::onSearchFilterChanged);
    connect(annotList, &QListWidget::currentRowChanged, this, &AnnotationPanel::onListSelectionChanged);
}

void AnnotationPanel::createEditorGroup() {
    QGroupBox* editor = new QGroupBox("Editor", this);
    editor->setObjectName("editorGroup");
    QVBoxLayout* mainLayout = new QVBoxLayout(editor);
    
    selectionLabel = new QLabel("New annotation");
    selectionLabel->setStyleSheet(QString("font-weight: bold; color: %1; font-size: 14px;").arg(theme::ACCENT_MINT));
    posLabel = new QLabel("Position: (Not selected)");
    posLabel->setStyleSheet(QString("color: %1; font-size: 12px;").arg(theme::TEXT_SECONDARY));
    
    mainLayout->addWidget(selectionLabel);
    mainLayout->addWidget(posLabel);
    mainLayout->addSpacing(8);
    
    QGridLayout* layout = new QGridLayout();
    layout->setColumnStretch(1, 1);
    
    nameEdit = new QLineEdit();
    
    kingdomRow = new QWidget();
    QHBoxLayout* kr = new QHBoxLayout(kingdomRow);
    kr->setContentsMargins(0, 0, 0, 0);
    kr->addWidget(new QLabel("Kingdom"));
    kingdomCb = new QComboBox();
    kr->addWidget(kingdomCb, 1);
    QPushButton* addKingdomBtn = new QPushButton("Add");
    kr->addWidget(addKingdomBtn);
    
    influenceRow = new QWidget();
    QHBoxLayout* ir = new QHBoxLayout(influenceRow);
    ir->setContentsMargins(0, 0, 0, 0);
    ir->addWidget(new QLabel("Influence Radius"));
    influenceSpin = new QSpinBox();
    influenceSpin->setRange(1, 500);
    ir->addWidget(influenceSpin);
    ir->addWidget(new QLabel("tiles"));
    ir->addStretch(1);
    
    statusCb = new QCheckBox("Old/Dead Village");
    notesEdit = new QTextEdit();
    notesEdit->setMaximumHeight(80); // Keep it compact
    
    layout->addWidget(new QLabel("Name:"), 0, 0);
    layout->addWidget(nameEdit, 0, 1);
    
    layout->addWidget(kingdomRow, 1, 0, 1, 2);
    layout->addWidget(influenceRow, 2, 0, 1, 2);
    
    layout->addWidget(statusCb, 3, 1);
    
    QLabel* notesLbl = new QLabel("Notes:");
    notesLbl->setAlignment(Qt::AlignTop);
    layout->addWidget(notesLbl, 4, 0);
    layout->addWidget(notesEdit, 4, 1);
    
    mainLayout->addLayout(layout);
    mainLayout->addSpacing(12);
    
    // Action Buttons
    saveBtn = new QPushButton("Save Annotation");
    saveBtn->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; border-radius: 4px; padding: 6px;").arg(theme::ACCENT_EMERALD, theme::TEXT_ON_ACCENT));
    undoBtn = new QPushButton("Undo Point");
    undoBtn->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 6px;").arg(theme::SURFACE_CARD, theme::TEXT_PRIMARY, theme::BORDER_MUTED));
    newBtn = new QPushButton("New / Clear");
    newBtn->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 6px;").arg(theme::SURFACE_CARD, theme::TEXT_PRIMARY, theme::BORDER_MUTED));
    deleteBtn = new QPushButton("Delete");
    deleteBtn->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; border-radius: 4px; padding: 6px;").arg(theme::STATUS_DANGER, theme::TEXT_PRIMARY));
    
    QGridLayout* btnGrid = new QGridLayout();
    btnGrid->addWidget(saveBtn, 0, 0);
    btnGrid->addWidget(undoBtn, 0, 1);
    btnGrid->addWidget(newBtn, 1, 0);
    btnGrid->addWidget(deleteBtn, 1, 1);
    
    mainLayout->addLayout(btnGrid);
    
    connect(saveBtn, &QPushButton::clicked, this, &AnnotationPanel::saveCurrent);
    connect(undoBtn, &QPushButton::clicked, this, &AnnotationPanel::undoPoint);
    connect(newBtn, &QPushButton::clicked, this, &AnnotationPanel::resetEditor);
    connect(deleteBtn, &QPushButton::clicked, this, &AnnotationPanel::deleteCurrent);
}

QPushButton* AnnotationPanel::createToolButton(const QString& text, const QString& typeData, bool checked) {
    QPushButton* btn = new QPushButton(text);
    btn->setCheckable(true);
    btn->setProperty("toolType", typeData);
    btn->setChecked(checked);
    
    // Segmented button styling
    btn->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 6px;
            color: %3;
            font-size: 12px;
            padding: 6px 4px;
        }
        QPushButton:hover {
            background-color: %4;
            border: 1px solid %5;
        }
        QPushButton:checked {
            background-color: %6;
            color: %7;
            border: 1px solid %8;
            font-weight: bold;
        }
    )").arg(theme::SURFACE_CARD, theme::BORDER_MUTED, theme::TEXT_PRIMARY, theme::SURFACE_HOVER, theme::ACCENT_EMERALD, theme::ACCENT_TINT, theme::ACCENT_MINT, theme::ACCENT_EMERALD));
    
    toolGroup->addButton(btn);
    connect(btn, &QPushButton::toggled, this, [this](bool isChecked){
        if (isChecked) onToolChange();
    });
    return btn;
}

void AnnotationPanel::setContext(std::shared_ptr<core::ServerConfig> cfg) {
    currentCfg = cfg;
    refreshList();
}

void AnnotationPanel::refreshList() {
    annotList->clear();
    listboxAnnots.clear();
    if (!annotStore || !currentCfg) {
        countLabel->setText("0 shown");
        return;
    }
    
    QString searchTxt = searchEdit->text().toLower();
    QString filterTxt = filterCb->currentText();
    
    auto allAnnots = annotStore->getManualByServer(currentCfg->name);
    
    for (const auto& a : allAnnots) {
        if (!searchTxt.isEmpty()) {
            if (QString::fromStdString(a.name).toLower().indexOf(searchTxt) == -1 &&
                QString::fromStdString(a.notes).toLower().indexOf(searchTxt) == -1) {
                continue;
            }
        }
        if (filterTxt != "All") {
            if (QString::fromStdString(a.type).compare(filterTxt, Qt::CaseInsensitive) != 0) {
                continue;
            }
        }
        listboxAnnots.push_back(a);
        annotList->addItem(QString::fromStdString(a.name) + " (" + QString::fromStdString(a.type) + ")");
    }
    countLabel->setText(QString::number(listboxAnnots.size()) + " shown");
    emit dataChanged();
}

void AnnotationPanel::onToolChange() {
    QAbstractButton* checked = toolGroup->checkedButton();
    if (checked) {
        draftType = checked->property("toolType").toString().toStdString();
    } else {
        draftType = "";
    }
}

void AnnotationPanel::onSearchFilterChanged() {
    refreshList();
}

void AnnotationPanel::onListSelectionChanged() {
    int idx = annotList->currentRow();
    if (idx >= 0 && static_cast<size_t>(idx) < listboxAnnots.size()) {
        const auto& a = listboxAnnots[idx];
        currentAnnotId = a.id;
        nameEdit->setText(QString::fromStdString(a.name));
        notesEdit->setText(QString::fromStdString(a.notes));
        statusCb->setChecked(a.status == "old");
        
        selectionLabel->setText("Editing: " + QString::fromStdString(a.name));
        
        if (a.type == "deed") {
            kingdomRow->show();
            influenceRow->show();
            // Need kingdom store
            influenceSpin->setValue(a.influence_radius);
        } else {
            kingdomRow->hide();
            influenceRow->hide();
        }
        
        if (!a.points.empty()) {
            posLabel->setText(QString("Position: (%1, %2)").arg(a.points[0].x).arg(a.points[0].y));
        } else {
            posLabel->setText("Position: (None)");
        }
    } else {
        resetEditor();
    }
}

void AnnotationPanel::onKingdomChanged() {
    // Update kingdom
}

void AnnotationPanel::saveCurrent() {
    if (!currentCfg) return;
    
    models::Annotation a;
    if (!currentAnnotId.empty()) {
        a.id = currentAnnotId;
    } else {
        a.id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    }
    
    a.server = currentCfg->name;
    a.name = nameEdit->text().toStdString();
    a.notes = notesEdit->toPlainText().toStdString();
    a.status = statusCb->isChecked() ? "old" : "current";
    a.points = pendingPoints;
    a.source = "manual";
    
    if (draftType.empty()) draftType = "deed";
    
    // Map draft tools back to type
    if (draftType == "add_deed") a.type = "deed";
    else if (draftType == "add_road") a.type = "road";
    else if (draftType == "add_bridge") a.type = "bridge";
    else if (draftType == "add_tunnel") a.type = "tunnel";
    else if (draftType == "add_guard_tower") a.type = "guard_tower";
    else a.type = draftType;
    
    if (a.type == "deed") {
        a.influence_radius = influenceSpin->value();
        a.kingdom = kingdomCb->currentText().toStdString();
    }
    
    if (currentAnnotId.empty()) {
        annotStore->add(a);
    } else {
        annotStore->update(a);
    }
    
    resetEditor();
    refreshList();
    emit dataChanged();
}

void AnnotationPanel::undoPoint() {
    if (!pendingPoints.empty()) {
        pendingPoints.pop_back();
        emit dataChanged();
    }
}

void AnnotationPanel::resetEditor() {
    currentAnnotId = "";
    pendingPoints.clear();
    nameEdit->clear();
    notesEdit->clear();
    statusCb->setChecked(false);
    selectionLabel->setText("New annotation");
    posLabel->setText("Position: (Not selected)");
    annotList->clearSelection();
    emit dataChanged();
}

void AnnotationPanel::deleteCurrent() {
    if (!currentAnnotId.empty()) {
        annotStore->remove(currentAnnotId);
        resetEditor();
        refreshList();
        emit dataChanged();
    }
}

} // namespace ui
} // namespace treasure
