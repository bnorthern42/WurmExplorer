#include "TreasurePanel.hpp"
#include "../../ui/ThemeTokens.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFrame>
#include <QFileDialog>

#include <QFormLayout>
#include <QScrollArea>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>

TreasurePanel::TreasurePanel(QWidget *parent) : QWidget(parent) {
    setupUi();
    connect(browseButton, &QPushButton::clicked, this, &TreasurePanel::onBrowseClicked);
    connect(pasteButton, &QPushButton::clicked, this, &TreasurePanel::onPasteClicked);
    connect(locateButton, &QPushButton::clicked, this, &TreasurePanel::onLocateClicked);
    connect(setRoiButton, &QPushButton::clicked, this, &TreasurePanel::onSetRoiClicked);
    connect(clearRoiButton, &QPushButton::clicked, this, &TreasurePanel::onClearRoiClicked);
    connect(matchListView, &QListWidget::currentRowChanged, this, &TreasurePanel::onMatchListItemClicked);
}

int TreasurePanel::getCanny1() const { return canny1Spin->value(); }
int TreasurePanel::getCanny2() const { return canny2Spin->value(); }
int TreasurePanel::getBlurSize() const { return blurSpin->value(); }
int TreasurePanel::getQl() const { return qlSpin->value(); }
float TreasurePanel::getEdgeWeight() const { return edgeWeightSpin->value(); }
float TreasurePanel::getGrayWeight() const { return grayWeightSpin->value(); }

void TreasurePanel::clearDebug() { debugArea->clear(); }
void TreasurePanel::appendDebug(const QString& text) { debugArea->append(text); }

void TreasurePanel::onBrowseClicked() {
    QString path = QFileDialog::getOpenFileName(this, "Select Screenshot", "", "Images (*.png *.jpg *.bmp)", nullptr, QFileDialog::DontUseNativeDialog);
    if (!path.isEmpty()) {
        screenshotPathEdit->setText(path);
    }
}

void TreasurePanel::onPasteClicked() {
    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if (mimeData && mimeData->hasImage()) {
        QImage image = qvariant_cast<QImage>(mimeData->imageData());
        if (!image.isNull()) {
            if (image.width() > 5000 || image.height() > 5000) {
                appendDebug("Error: Pasted image is too large (" + QString::number(image.width()) + "x" + QString::number(image.height()) + "). Did you paste a full map instead of a screenshot?");
                return;
            }
            QString tempPath = "configs/clipboard_temp.png";
            if (image.save(tempPath)) {
                screenshotPathEdit->setText(tempPath);
                appendDebug("Image pasted from clipboard and saved to " + tempPath);
            } else {
                appendDebug("Failed to save pasted image.");
            }
        }
    } else {
        appendDebug("No image found in clipboard.");
    }
}

void TreasurePanel::onLocateClicked() {
    if (!screenshotPathEdit->text().isEmpty()) {
        emit locateRequested(screenshotPathEdit->text());
    }
}

void TreasurePanel::onSetRoiClicked() {
    emit setRoiModeRequested(true);
    appendDebug("Draw a rectangle on the map to set the search area.");
}

void TreasurePanel::onClearRoiClicked() {
    emit setRoiModeRequested(false);
    appendDebug("Cleared search area. Searching full map.");
}

void TreasurePanel::onMatchListItemClicked(int row) {
    if (row >= 0 && row < static_cast<int>(currentMatches.size())) {
        emit matchSelected(row);
    }
}

void TreasurePanel::setMatchResults(const std::vector<treasure::cv::MatchResult>& results) {
    currentMatches = results;
    matchListView->clear();
    
    for (size_t i = 0; i < results.size(); ++i) {
        int scorePct = static_cast<int>(results[i].score * 100);
        QString itemText = QString("Match %1: %2%").arg(i + 1).arg(scorePct);
        matchListView->addItem(itemText);
    }
}

void TreasurePanel::setupUi() {
    using namespace treasure::ui;
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(16);

    auto* titleLabel = new QLabel("Treasure Locator", this);
    titleLabel->setStyleSheet(QString("font-size: 20px; font-weight: bold; color: %1;").arg(theme::ACCENT_MINT));
    layout->addWidget(titleLabel);

    auto* instructionLabel = new QLabel("Select a screenshot of your prospect result to locate the treasure.", this);
    instructionLabel->setWordWrap(true);
    instructionLabel->setStyleSheet(QString("color: %1;").arg(theme::TEXT_SECONDARY));
    layout->addWidget(instructionLabel);

    // Screenshot Selection Row
    auto* screenshotLayout = new QHBoxLayout();
    screenshotPathEdit = new QLineEdit(this);
    screenshotPathEdit->setPlaceholderText("Path to screenshot...");
    browseButton = new QPushButton("Browse...", this);
    pasteButton = new QPushButton("Paste 📋", this);
    screenshotLayout->addWidget(screenshotPathEdit);
    screenshotLayout->addWidget(browseButton);
    screenshotLayout->addWidget(pasteButton);
    layout->addLayout(screenshotLayout);

    // Locate Action
    locateButton = new QPushButton("Locate Treasure", this);
    locateButton->setMinimumHeight(44);
    locateButton->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: %2;
            font-weight: bold;
            font-size: 14px;
            border-radius: 8px;
            border: none;
        }
        QPushButton:hover {
            background-color: %3;
        }
        QPushButton:pressed {
            background-color: %4;
        }
    )").arg(theme::ACCENT_EMERALD, theme::TEXT_ON_ACCENT, theme::ACCENT_MINT, theme::ACCENT_PRESSED));
    
    auto* actionLayout = new QVBoxLayout();
    
    // Top row of action: Set Area / Clear Area
    auto* roiLayout = new QHBoxLayout();
    setRoiButton = new QPushButton("Set Area", this);
    clearRoiButton = new QPushButton("Clear Area", this);
    roiLayout->addWidget(setRoiButton);
    roiLayout->addWidget(clearRoiButton);
    
    actionLayout->addWidget(locateButton);
    actionLayout->addLayout(roiLayout);
    layout->addLayout(actionLayout);

    // Matches List
    auto* matchesLabel = new QLabel("Top Matches:", this);
    layout->addWidget(matchesLabel);
    
    matchListView = new QListWidget(this);
    matchListView->setMaximumHeight(100);
    matchListView->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3; border-radius: 6px;").arg(theme::SURFACE_CARD, theme::TEXT_PRIMARY, theme::BORDER_MUTED));
    layout->addWidget(matchListView);

    // Advanced Options Group
    auto* optionsGroup = new QGroupBox("Advanced Options (Hyperparameters)", this);
    auto* formLayout = new QFormLayout(optionsGroup);
    
    canny1Spin = new QSpinBox(this); canny1Spin->setRange(10, 200); canny1Spin->setValue(40);
    canny1Spin->setToolTip("Lower threshold for Canny edge detection. Increasing this removes weak edges.");
    
    canny2Spin = new QSpinBox(this); canny2Spin->setRange(50, 400); canny2Spin->setValue(130);
    canny2Spin->setToolTip("Upper threshold for Canny edge detection. Increasing this requires stronger edges to be detected.");
    
    blurSpin = new QSpinBox(this); blurSpin->setRange(1, 15); blurSpin->setValue(5); blurSpin->setSingleStep(2);
    blurSpin->setToolTip("Kernel size for Gaussian Blur. Higher values smooth the image more, reducing noise but blurring details.");
    
    edgeWeightSpin = new QDoubleSpinBox(this); edgeWeightSpin->setRange(0.0, 2.0); edgeWeightSpin->setValue(1.0); edgeWeightSpin->setSingleStep(0.1);
    edgeWeightSpin->setToolTip("Weight given to edge matching. Higher values prioritize shape/contour matching.");
    
    grayWeightSpin = new QDoubleSpinBox(this); grayWeightSpin->setRange(0.0, 2.0); grayWeightSpin->setValue(0.0); grayWeightSpin->setSingleStep(0.1);
    grayWeightSpin->setToolTip("Weight given to color/grayscale matching. Higher values prioritize matching the water/terrain brightness.");
    
    qlSpin = new QSpinBox(this); qlSpin->setRange(1, 100); qlSpin->setValue(50); qlSpin->setSingleStep(1);
    qlSpin->setToolTip("In-Game Map Quality Level. Adjusts scaling or expected features based on map quality.");
    
    formLayout->addRow("Canny Low:", canny1Spin);
    formLayout->addRow("Canny High:", canny2Spin);
    formLayout->addRow("Blur K-Size:", blurSpin);
    formLayout->addRow("Edge Weight:", edgeWeightSpin);
    formLayout->addRow("Color Weight:", grayWeightSpin);
    formLayout->addRow("In-Game Map QL:", qlSpin);
    layout->addWidget(optionsGroup);

    // Debug Console
    auto* debugLabel = new QLabel("Debug & Progress", this);
    layout->addWidget(debugLabel);
    
    debugArea = new QTextEdit(this);
    debugArea->setReadOnly(true);
    debugArea->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3; border-radius: 6px; font-family: monospace;").arg(theme::SURFACE_CARD, theme::TEXT_SECONDARY, theme::BORDER_MUTED));
    layout->addWidget(debugArea);

    // Spacer
    layout->addStretch(1);
}
