#include "MainWindow.hpp"
#include "components/NavigationBar.hpp"
#include "components/TopControlBar.hpp"
#include "../features/tools/ToolsPanel.hpp"
#include "../features/skills/SkillsPanel.hpp"
#include "../features/granger/GrangerPanel.hpp"
#include "../ui/panels/SailingPanel.hpp"
#include "../engine/ZoomMapCanvas.hpp"
#include <QApplication>
#include <QDir>
#include <QPixmap>
#include <QStackedWidget>
#include <QComboBox>
#include <iostream>

void MainWindow::captureDocScreenshots(const QString& outputDir) {
    QDir().mkpath(outputDir);
    resize(2560, 1440);
    showFullScreen();
    show();
    for (int i = 0; i < 20; ++i) {
        qApp->processEvents();
    }

    auto capture = [this, &outputDir](const QString& filename) {
        for (int i = 0; i < 20; ++i) {
            qApp->processEvents();
        }
        QPixmap pm = grab();
        QString fullPath = outputDir + "/" + filename;
        pm.save(fullPath, "PNG");
        std::cout << "Captured doc screenshot: " << fullPath.toStdString()
                  << " (" << pm.width() << "x" << pm.height() << ")" << std::endl;
    };

    // 1. Main Cartography & Navigation UI
    navBar->selectTab(0);
    capture("main_ui.png");

    // 2. Sailing Routes & Cluster Navigator (SFI / Southern Freedom Isles)
    if (topBar) {
        auto* clusterCombo = topBar->findChild<QComboBox*>("clusterCombo");
        if (clusterCombo) {
            int sfiIdx = clusterCombo->findText("Southern");
            if (sfiIdx >= 0) {
                clusterCombo->setCurrentIndex(sfiIdx);
            }
        }
    }
    navBar->selectTab(5);
    updateSailingContext();
    capture("sailing_cluster.png");

    // 3. Granger Livestock & Husbandry Evaluator
    navBar->selectTab(6);
    capture("livestock.png");

    // 4. Skills Monitor & Real-Time Log Syncer
    navBar->selectTab(7);
    capture("skills_tracker.png");

    // 5. Tools & Simulators - Imp Calculator
    navBar->selectTab(8);
    if (toolsPanel) toolsPanel->selectToolTab(0);
    capture("imp_calc.png");

    // 6. Tools & Simulators - Bridge Dirt Pillar Calculator
    if (toolsPanel) toolsPanel->selectToolTab(1);
    capture("bridge_pillar.png");

    // 7. Tools & Simulators - Mechanics Grinder Simulation Engine
    if (toolsPanel) toolsPanel->selectToolTab(2);
    capture("grinder.png");
}
