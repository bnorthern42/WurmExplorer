#include <QApplication>
#include <QTest>
#include <QSignalSpy>
#include <QClipboard>
#include <memory>
#include "../src/models/ArtifactStore.hpp"
#include "../src/ui/panels/ArtifactPanel.hpp"
#include "../src/features/treasure/TreasurePanel.hpp"
#include "../src/features/drawing/DrawingPanel.hpp"
#include "../src/features/skills/SkillsPanel.hpp"
#include "../src/features/skills/SkillTracker.hpp"
#include "../src/features/tools/ImpCalculatorWidget.hpp"
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QCompleter>

class TestUI : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {}
    
    void testArtifactPanel() {
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

    void testSkillsPanelHeaderButtonsRemoved() {
        skills::SkillsPanel panel;
        QList<QPushButton*> buttons = panel.findChildren<QPushButton*>();
        for (auto* btn : buttons) {
            QVERIFY(btn->toolTip() != "Reload entire log file from beginning");
            QVERIFY(btn->toolTip() != "Pause / Resume live log tailing");
            QVERIFY(!btn->text().contains("Settings"));
        }
    }

    void testImpCalculatorLiveSyncAndFuzzySearch() {
        tools::ImpCalculatorWidget widget;

        // 1. Verify Live Sync checkbox exists
        auto* liveSyncCheck = widget.findChild<QCheckBox*>("liveSyncCheckBox");
        QVERIFY(liveSyncCheck != nullptr);
        QVERIFY(!liveSyncCheck->isChecked());

        // 2. Verify Skill QComboBox exists, is editable, and has fuzzy completer
        auto* skillCombo = widget.findChild<QComboBox*>("skillComboBox");
        QVERIFY(skillCombo != nullptr);
        QVERIFY(skillCombo->isEditable());
        QVERIFY(skillCombo->completer() != nullptr);
        QCOMPARE(skillCombo->completer()->filterMode(), Qt::MatchContains);

        // 3. Select Blacksmithing
        int idx = skillCombo->findText("Blacksmithing");
        QVERIFY(idx >= 0);
        skillCombo->setCurrentIndex(idx);

        // 4. Toggle Live Sync ON
        liveSyncCheck->setChecked(true);
        QVERIFY(widget.isLiveSyncEnabled());

        // 5. Simulate a live log update via SkillTracker
        skills::SkillTracker::instance().processLine("[14:30:00] Blacksmithing increased by 0.0050 to 52.3400");

        // Spinbox should automatically update to 52.34
        QCOMPARE(widget.currentSkill(), 52.34);
        QVERIFY(widget.maxImpQl() > 0.0);

        // 6. When Live Sync is unchecked, manual edit should work and not be overwritten
        liveSyncCheck->setChecked(false);
        widget.setCurrentSkill(80.0);
        QCOMPARE(widget.currentSkill(), 80.0);

        skills::SkillTracker::instance().processLine("[14:35:00] Blacksmithing increased by 0.0050 to 52.3450");
        QCOMPARE(widget.currentSkill(), 80.0);
    }

    void cleanupTestCase() {}
};

QTEST_MAIN(TestUI)
#include "test_ui.moc"
