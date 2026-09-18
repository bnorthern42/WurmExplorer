#pragma once

#include <QMainWindow>
#include <QFutureWatcher>
#include "core/Config.hpp"
#include "core/MapVariants.hpp"
#include "engine/cv/Matcher.hpp"
#include <vector>
#include <memory>

#include "panels/AnnotationPanel.hpp"
#include "panels/ArtifactPanel.hpp"
#include "panels/MapDataPanel.hpp"
#include "../engine/cv/Extractor.hpp"
#include "../models/AnnotationStore.hpp"
#include "../models/ArtifactStore.hpp"
#include "../models/DrawingStore.hpp"

class ZoomMapCanvas;
class NavigationBar;
class TopControlBar;
class QStackedWidget;
class TreasurePanel;
class DrawingPanel;
namespace treasure {
namespace models {
class ClusterDrawingStore;
}
namespace ui {
class AnnotationPanel;
class ArtifactPanel;
class MapDataPanel;
class SailingPanel;
}
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onLocateRequested(const QString& screenshotPath);
    void onLocateFinished();
    
    void onClusterChanged();
    void onServerChanged(const QString& serverName);
    void onMapTypeChanged(const QString& type);
    
    void onAnnotationsChanged();
    void onMapDataChanged();
    
    void onMatchSelected(int index);
    void onSailingImageChanged();

private:
    void setupUi();
    void setupShortcuts();
    void refreshActiveCanvas();
    void refreshStatusBar();

    TopControlBar* topBar;
    NavigationBar* navBar;
    QStackedWidget* panelStack;
    ZoomMapCanvas* mapCanvas;
    
    TreasurePanel* treasurePanel;
    DrawingPanel* drawingPanel;
    treasure::ui::AnnotationPanel* annotationPanel = nullptr;
    treasure::ui::ArtifactPanel* artifactPanel = nullptr;
    treasure::ui::MapDataPanel* mapDataPanel = nullptr;
    treasure::ui::SailingPanel* sailingPanel = nullptr;
    
    std::shared_ptr<treasure::models::AnnotationStore> annotStore;
    std::shared_ptr<treasure::models::ArtifactStore> artifactStore;
    std::shared_ptr<treasure::models::DrawingStore> drawingStore;
    std::shared_ptr<treasure::models::ClusterDrawingStore> clusterDrawingStore;
    
    QFutureWatcher<std::vector<treasure::cv::MatchResult>> watcher;
    std::vector<treasure::cv::MatchResult> m_lastResults;

    treasure::core::ConfigManager configManager;
    std::shared_ptr<treasure::core::ServerConfig> currentConfig;
    std::shared_ptr<treasure::core::ServerMapVariants> currentVariants;
};
