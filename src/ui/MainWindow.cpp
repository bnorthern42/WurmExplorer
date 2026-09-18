#include "MainWindow.hpp"
#include "../engine/ZoomMapCanvas.hpp"
#include "components/TopControlBar.hpp"
#include "components/NavigationBar.hpp"
#include "../features/treasure/TreasurePanel.hpp"
#include "../features/drawing/DrawingPanel.hpp"
#include "panels/AnnotationPanel.hpp"
#include "panels/ArtifactPanel.hpp"
#include "panels/MapDataPanel.hpp"
#include "panels/SailingPanel.hpp"
#include "../engine/cv/Extractor.hpp"
#include "../engine/cv/Matcher.hpp"
#include "../engine/cv/Ocr.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QWidget>
#include <QKeySequence>
#include <QShortcut>
#include <QLabel>
#include <QComboBox>
#include <QtConcurrent>
#include <QMessageBox>
#include <QFuture>
#include <QStatusBar>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), 
      topBar(new TopControlBar(this)),
      navBar(new NavigationBar(this)),
      panelStack(new QStackedWidget(this)),
      mapCanvas(new ZoomMapCanvas(this)) {
          
    annotStore = std::make_shared<treasure::models::AnnotationStore>("configs/annotations.json");
    artifactStore = std::make_shared<treasure::models::ArtifactStore>("configs/artifacts.json");
    drawingStore = std::make_shared<treasure::models::DrawingStore>("configs/drawings.json");
    clusterDrawingStore = std::make_shared<treasure::models::ClusterDrawingStore>("configs/cluster_drawings.json");
    
    setupUi();
    
    // Attempt to load configs
    if (configManager.loadConfig("configs/servers.yaml")) {
        auto clusters = configManager.getAvailableClusters();
        topBar->setClusters(clusters);
        
        QSettings settings("WurmLocator", "Settings");
        QString savedCluster = settings.value("activeCluster").toString();
        QString savedServer = settings.value("activeServer").toString();
        QString savedMapType = settings.value("activeMapLayer").toString();
        int savedTab = settings.value("activeTab", 0).toInt();
        
        QString targetCluster = clusters.empty() ? "" : QString::fromStdString(clusters[0]);
        if (!savedCluster.isEmpty() && std::find(clusters.begin(), clusters.end(), savedCluster.toStdString()) != clusters.end()) {
            targetCluster = savedCluster;
        }
        
        if (!targetCluster.isEmpty()) {
            // This emits clusterChanged and updates servers
            int idx = topBar->findChild<QComboBox*>("clusterCombo")->findText(targetCluster);
            if (idx >= 0) topBar->findChild<QComboBox*>("clusterCombo")->setCurrentIndex(idx);
            
            auto servers = configManager.getServersForCluster(targetCluster.toStdString());
            topBar->setServers(servers);
            QString targetServer = servers.empty() ? "" : QString::fromStdString(servers[0]);
            if (!savedServer.isEmpty() && std::find(servers.begin(), servers.end(), savedServer.toStdString()) != servers.end()) {
                targetServer = savedServer;
            }
            
            if (!targetServer.isEmpty()) {
                int sIdx = topBar->findChild<QComboBox*>("serverCombo")->findText(targetServer);
                if (sIdx >= 0) topBar->findChild<QComboBox*>("serverCombo")->setCurrentIndex(sIdx);
                onServerChanged(targetServer);
            }
            
            if (!savedMapType.isEmpty()) {
                topBar->setCurrentMapType(savedMapType);
                onMapTypeChanged(savedMapType);
            }
        }
        
        QListWidget* navMenu = navBar->findChild<QListWidget*>("navMenu");
        if (navMenu) {
            // Force a signal emission by temporarily changing the row
            navMenu->setCurrentRow(-1);
            navMenu->setCurrentRow(savedTab);
        }
    }
    
    connect(treasurePanel, &TreasurePanel::locateRequested, this, &MainWindow::onLocateRequested);
    connect(&watcher, &QFutureWatcher<std::vector<treasure::cv::MatchResult>>::finished, this, &MainWindow::onLocateFinished);
    
    connect(topBar, &TopControlBar::clusterChanged, this, &MainWindow::onClusterChanged);
    connect(topBar, &TopControlBar::serverChanged, this, &MainWindow::onServerChanged);
    connect(topBar, &TopControlBar::mapTypeChanged, this, &MainWindow::onMapTypeChanged);
    connect(topBar, &TopControlBar::quitRequested, this, &MainWindow::close);
}

MainWindow::~MainWindow() = default;

void MainWindow::onLocateRequested(const QString& screenshotPath) {
    ::cv::Mat mapMat = mapCanvas->getMapMat();
    if (mapMat.empty()) {
        QMessageBox::warning(this, "Error", "No map loaded on canvas.");
        return;
    }
    
    ::cv::Mat screenshotBgr = ::cv::imread(screenshotPath.toStdString());
    if (screenshotBgr.empty()) {
        QMessageBox::warning(this, "Error", "Failed to load screenshot.");
        return;
    }

    // Mathematically ideal scale is 0.20 (100 tiles mapped to 500 pixels vs 1 pixel/tile topo).
    // We search a tight cluster around 0.20 to account for Extractor cropping inaccuracies.
    std::vector<float> scales = {0.18f, 0.19f, 0.20f, 0.21f, 0.22f};
    
    int mapQl = treasurePanel->getQl();
    std::vector<float> angles = {0.0f};
    
    // In-game QL > 80 (ql < 20 in generator) applies a random rotation
    if (mapQl > 80) {
        // rotationAmount = Server.rand.nextGaussian() * Math.toRadians(25 - (100 - ql));
        float stdDev = 25.0f - (100.0f - mapQl); // e.g. QL 99 -> 24 deg std dev
        float maxAngle = stdDev * 3.0f; // 99.7% confidence interval
        
        angles.clear();
        // Scan every 10 degrees within the distribution to balance performance and accuracy
        for (float a = -maxAngle; a <= maxAngle; a += 10.0f) {
            angles.push_back(a);
        }
    }

    int canny1 = treasurePanel->getCanny1();
    int canny2 = treasurePanel->getCanny2();
    int blurSize = treasurePanel->getBlurSize();
    float edgeW = treasurePanel->getEdgeWeight();
    float grayW = treasurePanel->getGrayWeight();

    TreasurePanel* tp = treasurePanel;
    tp->clearDebug();
    
    QString currentServer = currentConfig ? QString::fromStdString(currentConfig->name) : "Unknown";
    QString currentLayer = topBar->currentMapType();
    tp->appendDebug(QString("Target: Scanning %1 (%2)").arg(currentServer).arg(currentLayer));
    tp->appendDebug("Starting extraction and matching...");
    
    ::cv::Rect2i manualRoi;
    bool hasManualRoi = mapCanvas->getRoi(manualRoi);
    if (hasManualRoi) {
        tp->appendDebug("Using manual ROI for search.");
    }

    // Run the extraction and matching in a background thread
    QFuture<std::vector<treasure::cv::MatchResult>> future = QtConcurrent::run([screenshotBgr, mapMat, scales, angles, canny1, canny2, blurSize, edgeW, grayW, tp, manualRoi, hasManualRoi]() -> std::vector<treasure::cv::MatchResult> {
        ::cv::Mat patch = treasure::cv::Extractor::extractMapSquareFromScreenshot(screenshotBgr);
        
        treasure::cv::Extractor::XDetectResult xResult;
        bool hasX = treasure::cv::Extractor::detectXCenter(patch, xResult);
        
        ::cv::Mat hsv, maskRed1, maskRed2, maskDark, maskToInpaint;
        ::cv::cvtColor(patch, hsv, ::cv::COLOR_BGR2HSV);
        
        // Find Red and Black/Dark Brown pixels
        ::cv::inRange(hsv, ::cv::Scalar(0, 70, 50), ::cv::Scalar(10, 255, 255), maskRed1);
        ::cv::inRange(hsv, ::cv::Scalar(170, 70, 50), ::cv::Scalar(180, 255, 255), maskRed2);
        ::cv::inRange(hsv, ::cv::Scalar(0, 0, 0), ::cv::Scalar(180, 255, 60), maskDark);
        
        ::cv::bitwise_or(maskRed1, maskRed2, maskToInpaint);
        ::cv::bitwise_or(maskToInpaint, maskDark, maskToInpaint);
        
        // Restrict inpainting to just the bottom text region and the X (if found) to avoid destroying map features
        ::cv::Mat restrictMask = ::cv::Mat::zeros(patch.size(), CV_8U);
        int ph = patch.rows;
        int pw = patch.cols;
        
        // Bottom 25% for text
        ::cv::Rect textRect(0, static_cast<int>(ph * 0.75), pw, ph - static_cast<int>(ph * 0.75));
        restrictMask(textRect).setTo(::cv::Scalar(255));
        
        if (hasX) {
            int xs = static_cast<int>(xResult.size * 1.5);
            int x0 = std::max(0, static_cast<int>(xResult.cx) - xs / 2);
            int y0 = std::max(0, static_cast<int>(xResult.cy) - xs / 2);
            int rw = std::min(pw - x0, xs);
            int rh = std::min(ph - y0, xs);
            ::cv::Rect xRect(x0, y0, rw, rh);
            restrictMask(xRect).setTo(::cv::Scalar(255));
        }
        
        ::cv::bitwise_and(maskToInpaint, restrictMask, maskToInpaint);
        
        ::cv::Mat kernel = ::cv::getStructuringElement(::cv::MORPH_RECT, ::cv::Size(5, 5));
        ::cv::dilate(maskToInpaint, maskToInpaint, kernel, ::cv::Point(-1, -1), 2);
        ::cv::inpaint(patch, maskToInpaint, patch, 5, ::cv::INPAINT_TELEA);
        
        // Phase 1: OCR Macro-Targeting
        std::string ocrText = treasure::cv::Ocr::extractProspectText(screenshotBgr);
        
        // Safely send OCR text to GUI thread
        QMetaObject::invokeMethod(tp, [tp, ocrText]() {
            tp->appendDebug("OCR Extracted Text:\n" + QString::fromStdString(ocrText));
        }, Qt::QueuedConnection);

        auto deedOpt = treasure::cv::Ocr::findDeedCoordinates(ocrText);
        
        ::cv::Rect2i* roiPtr = nullptr;
        ::cv::Rect2i roi;
        if (hasManualRoi) {
            roi = manualRoi;
            roiPtr = &roi;
        } else if (deedOpt.has_value()) {
            int cx = deedOpt.value().x;
            int cy = deedOpt.value().y;
            // 500x500 box around the deed
            roi = ::cv::Rect2i(cx - 250, cy - 250, 500, 500);
            roiPtr = &roi;
        }

        auto results = treasure::cv::Matcher::matchPatchMultiscaleTopK(mapMat, patch, scales, angles, canny1, canny2, blurSize, edgeW, grayW, 7, roiPtr);
        
        if (hasX) {
            for (auto& res : results) {
                // Adjust centerPx to point exactly to where the red X is located
                res.centerPx.x = res.topLeft.x + (xResult.cx * res.scale);
                res.centerPx.y = res.topLeft.y + (xResult.cy * res.scale);
            }
        }
        
        return results;
    });
    
    watcher.setFuture(future);
    
    // Optionally clear old markers
    mapCanvas->clearMarkers();
}

void MainWindow::onLocateFinished() {
    std::vector<treasure::cv::MatchResult> results = watcher.result();
    m_lastResults = results;
    
    if (results.empty()) {
        QMessageBox::information(this, "Result", "No matches found.");
        treasurePanel->appendDebug("Finished: No matches found.");
        treasurePanel->setMatchResults(results);
    } else {
        treasurePanel->setMatchResults(results);
        mapCanvas->setMarkers(results);
        
        // Auto-zoom to best match
        float bestX = results[0].centerPx.x;
        float bestY = results[0].centerPx.y;
        mapCanvas->centerOn(bestX, bestY);
        
        treasurePanel->appendDebug(QString("Finished! Best Match Score: %1%").arg(static_cast<int>(results[0].score * 100)));
    }
}

void MainWindow::onMatchSelected(int index) {
    if (index >= 0 && index < static_cast<int>(m_lastResults.size())) {
        float cx = m_lastResults[index].centerPx.x;
        float cy = m_lastResults[index].centerPx.y;
        mapCanvas->centerOn(cx, cy);
        
        // Temporarily clear all markers and draw only the selected one as top
        // OR just rely on centerOn. We'll rely on centerOn for now.
    }
}

void MainWindow::onClusterChanged() {
    QString cluster = topBar->currentCluster();
    QSettings settings("WurmLocator", "Settings");
    settings.setValue("activeCluster", cluster);
    
    auto servers = configManager.getServersForCluster(cluster.toStdString());
    topBar->setServers(servers);
    if (!servers.empty()) {
        onServerChanged(QString::fromStdString(servers[0]));
    }
}

void MainWindow::onServerChanged(const QString& serverName) {
    QSettings settings("WurmLocator", "Settings");
    settings.setValue("activeServer", serverName);
    
    currentConfig = configManager.getServerConfig(serverName.toStdString());
    if (currentConfig) {
        currentVariants = treasure::core::discoverServerMapVariants(currentConfig);
        if (currentVariants) {
            topBar->setMapTypes(currentVariants->availableTypes());
            QListWidget* navMenu = navBar->findChild<QListWidget*>("navMenu");
            int currentTab = navMenu ? navMenu->currentRow() : -1;
            
            if (currentTab == 0) {
                topBar->setCurrentMapType("topo");
                onMapTypeChanged("topo");
            } else {
                QString cm = topBar->currentMapType();
                if (cm.isEmpty() || currentVariants->pathFor(cm.toStdString()).empty()) {
                    cm = QString::fromStdString(currentVariants->default_type);
                }
                topBar->setCurrentMapType(cm);
                onMapTypeChanged(cm);
            }
        }
        
        annotationPanel->setContext(currentConfig);
        mapDataPanel->setContext(currentConfig);
        drawingPanel->setContext(currentConfig, topBar->currentMapType());
    }
}

void MainWindow::onMapTypeChanged(const QString& mapType) {
    QSettings settings("WurmLocator", "Settings");
    settings.setValue("activeMapLayer", mapType);
    
    if (currentVariants) {
        std::string path = currentVariants->pathFor(mapType.toStdString());
        if (!path.empty()) {
            static QString lastLoadedPath = "";
            if (lastLoadedPath != QString::fromStdString(path)) {
                mapCanvas->loadImage(QString::fromStdString(path));
                lastLoadedPath = QString::fromStdString(path);
                statusBar()->showMessage(QString("Loaded map: %1").arg(QString::fromStdString(path)));
            }
        }
    }
    drawingPanel->setContext(currentConfig, mapType);
}

void MainWindow::onSailingImageChanged() {
    if (panelStack->currentWidget() == sailingPanel) {
        QImage img = sailingPanel->getDisplayImage();
        if (!img.isNull()) {
            QPixmap pix = QPixmap::fromImage(img);
            QString tempPath = "/tmp/wurm_sailing_temp.png";
            pix.save(tempPath);
            mapCanvas->loadImage(tempPath);
            mapCanvas->setOverlays(sailingPanel->getOverlays());
        }
    }
}

void MainWindow::refreshActiveCanvas() {
    if (panelStack->currentWidget() == sailingPanel) {
        onSailingImageChanged();
        return;
    }
    
    if (currentVariants) {
        std::string path = currentVariants->pathFor(topBar->currentMapType().toStdString());
        if (!path.empty()) {
            mapCanvas->loadImage(QString::fromStdString(path));
        }
    }
    
    if (currentConfig) {
        auto annots = annotStore->getByServer(currentConfig->name);
        mapCanvas->setAnnotations(annots);
        
        auto drawings = drawingStore->getByServer(currentConfig->name);
        mapCanvas->setDrawings(drawings);
    }
}

void MainWindow::onAnnotationsChanged() {
    mapCanvas->setAnnotations(annotationPanel->getVisibleAnnotations());
}

void MainWindow::onMapDataChanged() {
    if (panelStack->currentIndex() == 5) {
        mapCanvas->setOverlays(sailingPanel->getOverlays());
    }
}

void MainWindow::setupUi() {
    auto* centralWidget = new QWidget(this);
    
    // Main horizontal layout (Sidebar + Rest)
    auto* mainHBox = new QHBoxLayout(centralWidget);
    mainHBox->setContentsMargins(0, 0, 0, 0);
    mainHBox->setSpacing(0);

    // Left Navigation Bar
    mainHBox->addWidget(navBar);

    // Right side (Top bar + content area)
    auto* rightWidget = new QWidget(this);
    auto* rightVBox = new QVBoxLayout(rightWidget);
    rightVBox->setContentsMargins(0, 0, 0, 0);
    rightVBox->setSpacing(0);

    // Top Control Bar
    rightVBox->addWidget(topBar);

    // Content Area (Panels + Canvas)
    auto* contentHBox = new QHBoxLayout();
    contentHBox->setContentsMargins(0, 0, 0, 0);
    contentHBox->setSpacing(0);

    // Setup Panels
    treasurePanel = new TreasurePanel(this);
    drawingPanel = new DrawingPanel(drawingStore, this);
    annotationPanel = new treasure::ui::AnnotationPanel(annotStore, this);
    artifactPanel = new treasure::ui::ArtifactPanel(artifactStore, this);
    mapDataPanel = new treasure::ui::MapDataPanel(annotStore, this);
    sailingPanel = new treasure::ui::SailingPanel(clusterDrawingStore, this);
    
    connect(annotationPanel, &treasure::ui::AnnotationPanel::dataChanged, this, &MainWindow::onAnnotationsChanged);
    connect(artifactPanel, &treasure::ui::ArtifactPanel::dataChanged, this, &MainWindow::onAnnotationsChanged); // Reuse canvas update for artifacts later
    connect(mapDataPanel, &treasure::ui::MapDataPanel::dataChanged, this, &MainWindow::onMapDataChanged);
    connect(sailingPanel, &treasure::ui::SailingPanel::dataChanged, this, &MainWindow::onMapDataChanged);
    
    connect(drawingPanel, &DrawingPanel::toolSelected, mapCanvas, &ZoomMapCanvas::setInteractionMode);
    connect(drawingPanel, &DrawingPanel::propertiesChanged, this, [this]() {
        mapCanvas->setDrawingWidth(drawingPanel->getLineWidth());
        mapCanvas->setDrawingColor(drawingPanel->getCurrentColor());
    });
    connect(drawingPanel, &DrawingPanel::requestRedraw, this, &MainWindow::refreshActiveCanvas);
    
    // Initialize defaults
    mapCanvas->setDrawingWidth(drawingPanel->getLineWidth());
    mapCanvas->setDrawingColor(drawingPanel->getCurrentColor());
    
    // Undo shortcut
    auto* undoShortcut = new QShortcut(QKeySequence::Undo, this);
    connect(undoShortcut, &QShortcut::activated, this, [this]() {
        if (panelStack->currentWidget() == drawingPanel) {
            drawingPanel->undoLast();
        }
    });
    
    // Eraser signal
    connect(mapCanvas, &ZoomMapCanvas::eraseRequested, this, [this](const QString& id) {
        drawingStore->remove(id.toStdString());
        drawingStore->save();
        drawingPanel->refreshList();
        refreshActiveCanvas();
    });
    
    // Treasure Panel Signals
    connect(treasurePanel, &TreasurePanel::setRoiModeRequested, this, [this](bool active) {
        if (active) {
            mapCanvas->setInteractionMode("Select ROI");
        } else {
            mapCanvas->setInteractionMode("Drag");
            mapCanvas->clearRoi();
        }
    });
    connect(treasurePanel, &TreasurePanel::matchSelected, this, &MainWindow::onMatchSelected);
    connect(mapCanvas, &ZoomMapCanvas::shapeDrawn, this, [this](const treasure::models::DrawingItem& item) {
        if (!currentConfig) return;
        
        treasure::models::DrawingObject obj;
        obj.id = QUuid::createUuid().toString().toStdString();
        obj.server = currentConfig->name;
        obj.plan_name = "Default";
        obj.layer = topBar->currentMapType().toStdString();
        obj.name = "Shape";
        obj.items.push_back(item);
        
        drawingStore->add(obj);
        drawingStore->save();
        
        refreshActiveCanvas();
        drawingPanel->refreshList();
    });
    connect(sailingPanel, &treasure::ui::SailingPanel::imageChanged, this, &MainWindow::onSailingImageChanged);
    connect(mapDataPanel, &treasure::ui::MapDataPanel::focusRequested, this, [this](float tx, float ty) {
        // Will implement panTo later
        statusBar()->showMessage(QString("Focus requested at %1, %2").arg(tx).arg(ty), 2000);
    });
    
    panelStack->addWidget(treasurePanel);
    panelStack->addWidget(drawingPanel);
    panelStack->addWidget(annotationPanel);
    panelStack->addWidget(artifactPanel);
    panelStack->addWidget(mapDataPanel);
    panelStack->addWidget(sailingPanel);
    panelStack->setFixedWidth(350); // Fixed width for tools

    // Add tabs to nav bar
    navBar->addTab("  Treasure", 0);
    navBar->addTab("  Drawing", 1);
    navBar->addTab("  Annotations", 2);
    navBar->addTab("  Artifacts", 3);
    navBar->addTab("  Map Data", 4);
    navBar->addTab("  Sailing", 5);
    
    connect(navBar, &NavigationBar::tabSelected, panelStack, &QStackedWidget::setCurrentIndex);
    connect(navBar, &NavigationBar::tabSelected, this, [this](int index) {
        QSettings settings("WurmLocator", "Settings");
        settings.setValue("activeTab", index);
        
        if (index != 1) { // Not Drawing Panel
            mapCanvas->setInteractionMode("Drag");
        }
        
        if (index == 0) { // Treasure Panel
            topBar->setCurrentMapType("topo");
            topBar->setMapTypeEnabled(false);
            onMapTypeChanged("topo");
        } else {
            topBar->setMapTypeEnabled(true);
            onMapTypeChanged(topBar->currentMapType());
        }
        
        if (index == 5) { // Sailing Panel
            panelStack->setCurrentWidget(sailingPanel);
            
            auto serverNames = configManager.getServersForCluster(topBar->currentCluster().toStdString());
            std::map<std::string, std::shared_ptr<treasure::core::ServerConfig>> cfgByName;
            for (const auto& sName : serverNames) {
                auto cfgPtr = configManager.getServerConfig(sName);
                if (cfgPtr) {
                    cfgByName[sName] = std::make_shared<treasure::core::ServerConfig>(*cfgPtr);
                }
            }
            
            std::string mapType = topBar->currentMapType().toStdString();
            std::map<std::string, std::string> mapImages;
            for (const auto& [name, cfg] : cfgByName) {
                std::string type = mapType;
                if (type != "terrain" && type != "topo" && type != "classic") type = "terrain";
                auto variants = treasure::core::discoverServerMapVariants(cfg);
                if (variants) {
                    std::string p = variants->pathFor(type);
                    if (!p.empty()) {
                        mapImages[name] = p;
                    }
                }
            }
            sailingPanel->setContext(cfgByName, mapType, mapImages);
            onSailingImageChanged(); // force image update to canvas
        }
    });
    navBar->selectTab(0); // Default to treasure

    contentHBox->addWidget(panelStack);
    contentHBox->addWidget(mapCanvas, 1); // Canvas takes remaining space

    rightVBox->addLayout(contentHBox);
    mainHBox->addWidget(rightWidget, 1);
    
    setCentralWidget(centralWidget);
    setWindowTitle("WurmExplorer (C++ Edition)");
    statusBar()->showMessage("Ready");
}
