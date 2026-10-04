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
#include "../features/granger/GrangerPanel.hpp"
#include "../features/granger/GrangerStore.hpp"
#include "../features/skills/SkillsPanel.hpp"
#include "../features/tools/ToolsPanel.hpp"
#include "../features/tools/GrinderWidget.hpp"
#include "dialogs/SettingsDialog.hpp"
#include "../engine/cv/Extractor.hpp"
#include "../engine/cv/Matcher.hpp"
#include "../engine/cv/Ocr.hpp"


#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QWidget>
#include <QKeySequence>
#include <QShortcut>
#include <QIcon>
#include <QLabel>
#include <QComboBox>
#include <QtConcurrent>
#include <QMessageBox>
#include <QFuture>
#include <QStatusBar>
#include <QSettings>
#include "../core/PathUtils.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), 
      topBar(new TopControlBar(this)),
      navBar(new NavigationBar(this)),
      panelStack(new QStackedWidget(this)),
      mapCanvas(new ZoomMapCanvas(this)) {
          
    annotStore = std::make_shared<treasure::models::AnnotationStore>(treasure::core::resolveStorePath("annotations.json"));
    artifactStore = std::make_shared<treasure::models::ArtifactStore>(treasure::core::resolveStorePath("artifacts.json"));
    drawingStore = std::make_shared<treasure::models::DrawingStore>(treasure::core::resolveStorePath("drawings.json"));
    clusterDrawingStore = std::make_shared<treasure::models::ClusterDrawingStore>(treasure::core::resolveStorePath("cluster_drawings.json"));
    grangerStore = std::make_shared<granger::GrangerStore>(treasure::core::resolveStorePath("breeding.json"));

    
    setupUi();
    
    // Attempt to load configs
    if (configManager.loadConfig(treasure::core::resolveServerConfigPath())) {
        auto clusters = configManager.getAvailableClusters();
        topBar->setClusters(clusters);
        
        QSettings settings("WurmExplorer", "Settings");
        QString savedCluster = settings.value("activeCluster").toString();
        if (savedCluster.isEmpty()) {
            QSettings legacy("WurmLocator", "Settings");
            savedCluster = legacy.value("activeCluster").toString();
        }
        QString savedServer = settings.value("activeServer").toString();
        if (savedServer.isEmpty()) {
            QSettings legacy("WurmLocator", "Settings");
            savedServer = legacy.value("activeServer").toString();
        }
        QString savedMapType = settings.value("activeMapLayer").toString();
        if (savedMapType.isEmpty()) {
            QSettings legacy("WurmLocator", "Settings");
            savedMapType = legacy.value("activeMapLayer").toString();
        }
        int savedTab = settings.value("activeTab", -1).toInt();
        if (savedTab == -1) {
            QSettings legacy("WurmLocator", "Settings");
            savedTab = legacy.value("activeTab", 0).toInt();
        }
        
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
        
        navBar->selectTab(savedTab);
    }
    
    connect(treasurePanel, &TreasurePanel::locateRequested, this, &MainWindow::onLocateRequested);
    connect(&watcher, &QFutureWatcher<std::vector<treasure::cv::MatchResult>>::finished, this, &MainWindow::onLocateFinished);
    
    connect(topBar, &TopControlBar::clusterChanged, this, &MainWindow::onClusterChanged);
    connect(topBar, &TopControlBar::serverChanged, this, &MainWindow::onServerChanged);
    connect(topBar, &TopControlBar::mapTypeChanged, this, &MainWindow::onMapTypeChanged);
    connect(topBar, &TopControlBar::quitRequested, this, &MainWindow::close);
}

MainWindow::~MainWindow() = default;

// Note: onLocateRequested, onLocateFinished, and onMatchSelected
// are defined in MainWindowLocate.cpp to keep file size under 500 lines.


void MainWindow::onClusterChanged() {
    QString cluster = topBar->currentCluster();
    QSettings settings("WurmExplorer", "Settings");
    settings.setValue("activeCluster", cluster);
    
    auto servers = configManager.getServersForCluster(cluster.toStdString());
    topBar->setServers(servers);
    if (!servers.empty()) {
        onServerChanged(QString::fromStdString(servers[0]));
    }
    
    if (navBar->currentTab() == 5) {
        updateSailingContext();
    }
}

void MainWindow::onServerChanged(const QString& serverName) {
    QSettings settings("WurmExplorer", "Settings");
    settings.setValue("activeServer", serverName);
    
    currentConfig = configManager.getServerConfig(serverName.toStdString());
    if (currentConfig) {
        currentVariants = treasure::core::discoverServerMapVariants(currentConfig);
        int currentTab = navBar->currentTab();
        if (currentVariants) {
            topBar->setMapTypes(currentVariants->availableTypes());
            
            if (currentTab == 0) {
                topBar->setCurrentMapType("topo");
                onMapTypeChanged("topo");
            } else if (currentTab != 5 && currentTab != 6 && currentTab != 7) {
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
        
        QString player = ui::SettingsDialog::getActivePlayer();
        navBar->setPlayerInfo(player, QString("● %1").arg(serverName));
        
        if (currentTab == 5) {
            updateSailingContext();
        }
    }
}

void MainWindow::onMapTypeChanged(const QString& mapType) {
    QSettings settings("WurmExplorer", "Settings");
    settings.setValue("activeMapLayer", mapType);
    
    if (navBar->currentTab() == 5) {
        updateSailingContext();
        return;
    }
    
    refreshActiveCanvas();
    drawingPanel->setContext(currentConfig, mapType);
}

void MainWindow::updateSailingContext() {
    QString cluster = topBar->currentCluster();
    auto serverNames = configManager.getServersForCluster(cluster.toStdString());
    std::map<std::string, std::shared_ptr<treasure::core::ServerConfig>> cfgByName;
    for (const auto& sName : serverNames) {
        auto cfgPtr = configManager.getServerConfig(sName);
        if (cfgPtr) {
            cfgByName[sName] = std::make_shared<treasure::core::ServerConfig>(*cfgPtr);
        }
    }
    
    std::string mapType = topBar->currentMapType().toStdString();
    if (mapType != "terrain" && mapType != "topo" && mapType != "classic") {
        mapType = "terrain";
    }
    
    std::map<std::string, std::string> mapImages;
    for (const auto& [name, cfg] : cfgByName) {
        auto variants = treasure::core::discoverServerMapVariants(cfg);
        if (variants) {
            std::string p = variants->pathFor(mapType);
            if (!p.empty()) {
                mapImages[name] = p;
            }
        }
    }
    sailingPanel->setContext(cfgByName, mapType, mapImages, cluster.toStdString());
    onSailingImageChanged();
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
    
    // Clear sailing overlays when on regular server maps
    mapCanvas->setOverlays({});
    
    if (currentVariants) {
        std::string path = currentVariants->pathFor(topBar->currentMapType().toStdString());
        if (!path.empty()) {
            mapCanvas->loadImage(QString::fromStdString(path));
            statusBar()->showMessage(QString("Loaded map: %1").arg(QString::fromStdString(path)));
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
    grangerPanel = new granger::GrangerPanel(grangerStore, this);
    skillsPanel = new skills::SkillsPanel(this);
    toolsPanel = new tools::ToolsPanel(this);
    connect(skillsPanel, &skills::SkillsPanel::skillsUpdated,
            toolsPanel->getGrinderWidget(), &tools::GrinderWidget::onSkillsUpdated);
    toolsPanel->getGrinderWidget()->onSkillsUpdated(skillsPanel->getStats());
    
    auto openSettings = [this]() {
        ui::SettingsDialog dlg(this);
        connect(&dlg, &ui::SettingsDialog::settingsSaved, skillsPanel, &skills::SkillsPanel::onSettingsSaved);
        if (dlg.exec() == QDialog::Accepted) {
            QString player = ui::SettingsDialog::getActivePlayer();
            QString sName = currentConfig ? QString::fromStdString(currentConfig->name) : "Online";
            navBar->setPlayerInfo(player, QString("● %1").arg(sName));
        }
    };
    connect(topBar, &TopControlBar::settingsRequested, this, openSettings);
    connect(navBar, &NavigationBar::settingsRequested, this, openSettings);
    
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
    panelStack->addWidget(grangerPanel);
    panelStack->addWidget(skillsPanel);
    panelStack->addWidget(toolsPanel);
    panelStack->setFixedWidth(350); // Fixed width for tools

    // Categorized Dashboard Navigation
    navBar->addSectionHeader("Cartography");
    navBar->addTab("  Treasure Locator", 0);
    navBar->addTab("  Map Drawing", 1);
    navBar->addTab("  Annotations", 2);
    navBar->addTab("  Artifact Clues", 3);
    navBar->addTab("  Imported Data", 4);
    navBar->addTab("  Sailing Routes", 5);

    navBar->addSectionHeader("Character");
    navBar->addTab("  Granger Livestock", 6);
    navBar->addTab("  Skills Monitor", 7);

    navBar->addSectionHeader("Workbench");
    navBar->addTab("  Tools & Simulators", 8);

    connect(navBar, &NavigationBar::tabSelected, panelStack, &QStackedWidget::setCurrentIndex);
    connect(navBar, &NavigationBar::tabSelected, this, [this](int index) {
        QSettings settings("WurmExplorer", "Settings");
        settings.setValue("activeTab", index);
        
        // Update Dashboard Breadcrumbs
        switch (index) {
            case 0: topBar->setContextBreadcrumb("Cartography", "Treasure Locator"); break;
            case 1: topBar->setContextBreadcrumb("Cartography", "Map Drawing"); break;
            case 2: topBar->setContextBreadcrumb("Cartography", "Annotations"); break;
            case 3: topBar->setContextBreadcrumb("Cartography", "Artifact Clues"); break;
            case 4: topBar->setContextBreadcrumb("Cartography", "Imported Map Data"); break;
            case 5: topBar->setContextBreadcrumb("Cartography", "Sailing Routes"); break;
            case 6: topBar->setContextBreadcrumb("Character", "Granger Livestock"); break;
            case 7: topBar->setContextBreadcrumb("Character", "Skills Tracker"); break;
            case 8: topBar->setContextBreadcrumb("Workbench", "Tools & Simulators"); break;
        }
        
        bool isMapTab = (index != 6 && index != 7 && index != 8);
        mapCanvas->setVisible(isMapTab);
        topBar->setMapControlsVisible(isMapTab);
        
        if (!isMapTab) {
            panelStack->setMinimumWidth(0);
            panelStack->setMaximumWidth(QWIDGETSIZE_MAX);
            panelStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        } else {
            panelStack->setFixedWidth(350);
            panelStack->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        }
        
        if (index != 1 && isMapTab) { // Not Drawing Panel
            mapCanvas->setInteractionMode("Drag");
        }
        
        if (index == 5) { // Sailing Panel
            panelStack->setCurrentWidget(sailingPanel);
            updateSailingContext();
        } else if (isMapTab) {
            if (index == 0) { // Treasure Panel
                topBar->setCurrentMapType("topo");
                topBar->setMapTypeEnabled(false);
            } else {
                topBar->setMapTypeEnabled(true);
            }
            refreshActiveCanvas();
            drawingPanel->setContext(currentConfig, topBar->currentMapType());
        }
    });
    navBar->selectTab(0); // Default to treasure

    contentHBox->addWidget(panelStack, 1);
    contentHBox->addWidget(mapCanvas, 1); // Canvas takes remaining space

    rightVBox->addLayout(contentHBox);
    mainHBox->addWidget(rightWidget, 1);
    
    setCentralWidget(centralWidget);
    setWindowTitle(QString("WurmExplorer v%1").arg(treasure::core::getAppVersion()));
    setWindowIcon(QIcon(treasure::core::resolveResourcePath("resources/wurmexplorer.svg")));
    statusBar()->showMessage("Ready");
}
