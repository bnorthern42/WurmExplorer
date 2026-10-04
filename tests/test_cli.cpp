#include <QTest>
#include <QObject>
#include <QStringList>
#include "../src/core/CliOptions.hpp"

class TestCli : public QObject {
    Q_OBJECT

private slots:
    void testDefaultOptions() {
        bool helpRequested = false;
        QString errorMsg;
        auto opts = treasure::core::parseCommandLine({"wurm_explorer"}, &helpRequested, &errorMsg);
        QVERIFY(!helpRequested);
        QVERIFY(errorMsg.isEmpty());
        QCOMPARE(opts.width, 2560);
        QCOMPARE(opts.height, 1440);
        QVERIFY(!opts.fullscreen);
        QVERIFY(!opts.maximized);
        QVERIFY(!opts.captureDocs);
        QCOMPARE(opts.initialTab, -1);
    }

    void testFullscreenFlags() {
        auto opts1 = treasure::core::parseCommandLine({"wurm_explorer", "--fullscreen"});
        QVERIFY(opts1.fullscreen);

        auto opts2 = treasure::core::parseCommandLine({"wurm_explorer", "-f"});
        QVERIFY(opts2.fullscreen);
    }

    void testMaximizedFlags() {
        auto opts1 = treasure::core::parseCommandLine({"wurm_explorer", "--maximized"});
        QVERIFY(opts1.maximized);

        auto opts2 = treasure::core::parseCommandLine({"wurm_explorer", "-m"});
        QVERIFY(opts2.maximized);
    }

    void testExplicitWidthAndHeight() {
        auto opts1 = treasure::core::parseCommandLine({"wurm_explorer", "--width", "1280", "--height", "720"});
        QCOMPARE(opts1.width, 1280);
        QCOMPARE(opts1.height, 720);
        QVERIFY(opts1.hasCustomSize);

        auto opts2 = treasure::core::parseCommandLine({"wurm_explorer", "-w", "1920", "-H", "1080"});
        QCOMPARE(opts2.width, 1920);
        QCOMPARE(opts2.height, 1080);
        QVERIFY(opts2.hasCustomSize);
    }

    void testSizeFlagFormats() {
        auto opts1 = treasure::core::parseCommandLine({"wurm_explorer", "--size", "1920x1080"});
        QCOMPARE(opts1.width, 1920);
        QCOMPARE(opts1.height, 1080);
        QVERIFY(opts1.hasCustomSize);

        auto opts2 = treasure::core::parseCommandLine({"wurm_explorer", "-s", "1280x720"});
        QCOMPARE(opts2.width, 1280);
        QCOMPARE(opts2.height, 720);
        QVERIFY(opts2.hasCustomSize);

        // Pre-defined shortcuts
        auto optsHalf = treasure::core::parseCommandLine({"wurm_explorer", "--size", "half"});
        QCOMPARE(optsHalf.width, 1280);
        QCOMPARE(optsHalf.height, 1440);
        QVERIFY(optsHalf.hasCustomSize);

        auto optsHalfFlag = treasure::core::parseCommandLine({"wurm_explorer", "--half-screen"});
        QCOMPARE(optsHalfFlag.width, 1280);
        QCOMPARE(optsHalfFlag.height, 1440);
        QVERIFY(optsHalfFlag.hasCustomSize);

        auto optsFull = treasure::core::parseCommandLine({"wurm_explorer", "--size", "full"});
        QVERIFY(optsFull.fullscreen);
    }

    void testCaptureDocsFlags() {
        auto opts1 = treasure::core::parseCommandLine({"wurm_explorer", "--capture-docs"});
        QVERIFY(opts1.captureDocs);
        QCOMPARE(opts1.captureDocsDir, QString("assets/docs"));

        auto opts2 = treasure::core::parseCommandLine({"wurm_explorer", "--capture-docs", "/tmp/docs_out"});
        QVERIFY(opts2.captureDocs);
        QCOMPARE(opts2.captureDocsDir, QString("/tmp/docs_out"));
    }

    void testTabFlags() {
        auto opts1 = treasure::core::parseCommandLine({"wurm_explorer", "--tab", "sailing"});
        QCOMPARE(opts1.initialTab, 5);

        auto opts2 = treasure::core::parseCommandLine({"wurm_explorer", "-t", "livestock"});
        QCOMPARE(opts2.initialTab, 6);

        auto opts3 = treasure::core::parseCommandLine({"wurm_explorer", "-t", "granger"});
        QCOMPARE(opts3.initialTab, 6);

        auto opts4 = treasure::core::parseCommandLine({"wurm_explorer", "-t", "tools"});
        QCOMPARE(opts4.initialTab, 8);

        auto opts5 = treasure::core::parseCommandLine({"wurm_explorer", "-t", "8"});
        QCOMPARE(opts5.initialTab, 8);
    }
};

QTEST_MAIN(TestCli)
#include "test_cli.moc"
