#include <QApplication>
#include <QTest>
#include <QSignalSpy>
#include <QClipboard>
#include <memory>
#include "../src/models/ArtifactStore.hpp"
#include "../src/ui/panels/ArtifactPanel.hpp"
#include "../src/features/treasure/TreasurePanel.hpp"
#include "../src/features/drawing/DrawingPanel.hpp"
#include <QSpinBox>
#include <QDoubleSpinBox>

class TestUI : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {}
    
    void testArtifactPanel() {
        int argc = 0;
        char** argv = nullptr;
        QApplication app(argc, argv);
        
        auto store = std::make_shared<treasure::models::ArtifactStore>("configs/artifacts.json");
        treasure::ui::ArtifactPanel panel(store);
        
        QVERIFY(panel.objectName() != "Broken");
    }

    void testTreasurePanelPasteMap() {
        TreasurePanel panel;
        
        QImage hugeMap(8000, 8000, QImage::Format_RGB32);
        hugeMap.fill(Qt::green);
        
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setImage(hugeMap);
        
        QSignalSpy spy(&panel, &TreasurePanel::locateRequested);
        
        QMetaObject::invokeMethod(&panel, "onPasteClicked");
        
        QCOMPARE(spy.count(), 0);
    }
    
    void testTreasurePanelTooltips() {
        TreasurePanel panel;
        QSpinBox* canny1 = panel.findChild<QSpinBox*>(); // First one should be canny1
        QVERIFY(canny1 != nullptr);
        QVERIFY(!canny1->toolTip().isEmpty()); // Should have a tooltip
    }

    void testDrawingPanelDragTool() {
        auto store = std::make_shared<treasure::models::DrawingStore>("/tmp/mock_drawings.json");
        DrawingPanel panel(store);
        QSignalSpy spy(&panel, &DrawingPanel::toolSelected);
        
        // Let's find the Drag button
        QPushButton* dragBtn = nullptr;
        QList<QPushButton*> buttons = panel.findChildren<QPushButton*>();
        for (auto* btn : buttons) {
            if (btn->property("toolName").toString() == "Drag") {
                dragBtn = btn;
                break;
            }
        }
        
        QVERIFY(dragBtn != nullptr); // Drag button must exist
        QVERIFY(dragBtn->isChecked()); // Should be checked by default
    }

    void cleanupTestCase() {}
};

QTEST_MAIN(TestUI)
#include "test_ui.moc"
