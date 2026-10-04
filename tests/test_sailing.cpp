#include <QApplication>
#include <QTest>
#include <QComboBox>
#include <memory>
#include <filesystem>
#include <map>
#include <string>

#include "../src/models/ClusterLayout.hpp"
#include "../src/models/ClusterDrawingStore.hpp"
#include "../src/ui/panels/SailingPanel.hpp"
#include "../src/core/Config.hpp"

class TestSailing : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {}

    void testNorthernClusterLayout() {
        std::map<std::string, int> serverSizes = {
            {"Cadence", 4096},
            {"Harmony", 4096},
            {"Melody", 2048},
            {"Defiance", 4096}
        };

        auto layout = treasure::models::SailingLogic::buildLayout(serverSizes, "Northern");
        QCOMPARE(layout.name, std::string("Northern"));
        QCOMPARE(layout.servers.size(), (size_t)4);
        QVERIFY(layout.servers.find("Cadence") != layout.servers.end());
        QVERIFY(layout.servers.find("Harmony") != layout.servers.end());
        QVERIFY(layout.servers.find("Melody") != layout.servers.end());
        QVERIFY(layout.servers.find("Defiance") != layout.servers.end());
        QVERIFY(layout.width_tiles > 0.0f);
        QVERIFY(layout.height_tiles > 0.0f);
    }

    void testEpicClusterLayout() {
        std::map<std::string, int> serverSizes = {
            {"Elevation", 2048},
            {"Desertion", 2048},
            {"Serenity", 2048},
            {"Affliction", 2048}
        };

        auto layout = treasure::models::SailingLogic::buildLayout(serverSizes, "Epic");
        QCOMPARE(layout.name, std::string("Epic"));
        QCOMPARE(layout.servers.size(), (size_t)4);
        QVERIFY(layout.servers.find("Elevation") != layout.servers.end());
        QVERIFY(layout.servers.find("Desertion") != layout.servers.end());
        QVERIFY(layout.servers.find("Serenity") != layout.servers.end());
        QVERIFY(layout.servers.find("Affliction") != layout.servers.end());
        QVERIFY(layout.width_tiles > 0.0f);
        QVERIFY(layout.height_tiles > 0.0f);
    }

    void testSailingPanelClusterSwitch() {
        std::string tempDb = "/tmp/test_cluster_store.json";
        if (std::filesystem::exists(tempDb)) {
            std::filesystem::remove(tempDb);
        }
        auto store = std::make_shared<treasure::models::ClusterDrawingStore>(tempDb);
        treasure::ui::SailingPanel panel(store);

        // Populate Northern cluster
        std::map<std::string, std::shared_ptr<treasure::core::ServerConfig>> northernCfgs;
        for (const auto& name : {"Cadence", "Harmony", "Melody", "Defiance"}) {
            auto cfg = std::make_shared<treasure::core::ServerConfig>();
            cfg->name = name;
            cfg->map_size_tiles = (name == std::string("Melody")) ? 2048 : 4096;
            northernCfgs[name] = cfg;
        }

        panel.setContext(northernCfgs, "terrain", {}, "Northern");

        // Verify that server combo boxes in panel have Northern servers
        auto combos = panel.findChildren<QComboBox*>();
        bool hasCadence = false;
        bool hasHarmony = false;
        for (auto* cb : combos) {
            if (cb->findText("Cadence") >= 0) hasCadence = true;
            if (cb->findText("Harmony") >= 0) hasHarmony = true;
        }
        QVERIFY(hasCadence);
        QVERIFY(hasHarmony);

        // Now switch to Southern cluster
        std::map<std::string, std::shared_ptr<treasure::core::ServerConfig>> southernCfgs;
        for (const auto& name : {"Chaos", "Independence", "Xanadu"}) {
            auto cfg = std::make_shared<treasure::core::ServerConfig>();
            cfg->name = name;
            cfg->map_size_tiles = (name == std::string("Xanadu")) ? 8192 : 4096;
            southernCfgs[name] = cfg;
        }

        panel.setContext(southernCfgs, "terrain", {}, "Southern");

        // Verify that server combo boxes now have Southern servers and not Cadence
        bool hasChaos = false;
        hasCadence = false;
        for (auto* cb : combos) {
            if (cb->findText("Chaos") >= 0) hasChaos = true;
            if (cb->findText("Cadence") >= 0) hasCadence = true;
        }
        QVERIFY(hasChaos);
        QVERIFY(!hasCadence);

        if (std::filesystem::exists(tempDb)) {
            std::filesystem::remove(tempDb);
        }
    }

    void cleanupTestCase() {}
};

QTEST_MAIN(TestSailing)
#include "test_sailing.moc"
