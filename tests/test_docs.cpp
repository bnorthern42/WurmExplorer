#include <QTest>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QStringList>

class TestDocs : public QObject {
    Q_OBJECT

private slots:
    void testReadmeCookingNote() {
        QFile file("README.md");
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), "Could not open README.md");
        QString content = QString::fromUtf8(file.readAll());
        file.close();

        // Must include polite/humorous note about cooking apps
        QVERIFY2(content.contains("cooking", Qt::CaseInsensitive), "README.md missing cooking note");
        QVERIFY2(content.contains("900", Qt::CaseInsensitive), "README.md missing reference to existing 900k cooking apps");
    }

    void testReadmeNoScreenshotAutomationSection() {
        QFile file("README.md");
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), "Could not open README.md");
        QString content = QString::fromUtf8(file.readAll());
        file.close();

        // Screenshot automation section should be removed from README
        QVERIFY2(!content.contains("Screenshot Automation (Wayland / Niri)"), "README.md still contains Screenshot Automation header");
        QVERIFY2(!content.contains("scripts/capture_docs.sh"), "README.md still references scripts/capture_docs.sh");
    }

    void testReadmeScreenshotsExist() {
        QStringList screenshots = {
            "assets/docs/main_ui.png",
            "assets/docs/sailing_cluster.png",
            "assets/docs/grinder.png",
            "assets/docs/bridge_pillar.png",
            "assets/docs/imp_calc.png",
            "assets/docs/livestock.png",
            "assets/docs/skills_tracker.png"
        };

        for (const QString& relPath : screenshots) {
            QFileInfo info(relPath);
            QVERIFY2(info.exists(), QString("Missing screenshot file: %1").arg(relPath).toUtf8().constData());
            QVERIFY2(info.size() > 1000, QString("Screenshot too small or empty: %1").arg(relPath).toUtf8().constData());

            QImage img(relPath);
            QVERIFY2(!img.isNull(), QString("Invalid image format: %1").arg(relPath).toUtf8().constData());
            QVERIFY2(img.width() >= 600, QString("Screenshot width too small: %1 (%2px)").arg(relPath).arg(img.width()).toUtf8().constData());
            QVERIFY2(img.height() >= 400, QString("Screenshot height too small: %1 (%2px)").arg(relPath).arg(img.height()).toUtf8().constData());
        }
    }
};

QTEST_MAIN(TestDocs)
#include "test_docs.moc"
