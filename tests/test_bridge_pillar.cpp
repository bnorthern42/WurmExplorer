#include <QTest>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include "../src/features/tools/BridgePillarCalculator.hpp"
#include "../src/features/tools/BridgePillarWidget.hpp"

using namespace tools;

class TestBridgePillar : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {}

    void testChebyshevSquareDropoff() {
        // Top 1x2 -> 1x2 plateau corners (NOT 2x3).
        // Target height: 1800, max slope: 300 (r = 6).
        auto res = BridgePillarCalculator::calculate(1, 2, 1800, std::nullopt, false);
        
        QCOMPARE(res.topW, 1);
        QCOMPARE(res.topL, 2);
        QCOMPARE(res.plateauCornersX, 1);
        QCOMPARE(res.plateauCornersY, 2);

        int r = res.spreadRadius;  // 6
        QCOMPARE(r, 6);
        QCOMPARE(res.cornerW, 13); // 1 + 2 * 6 = 13
        QCOMPARE(res.cornerL, 14); // 2 + 2 * 6 = 14

        int startX = r;            // 6
        int startY = r;            // 6
        int endX = startX + 1 - 1; // 6 (width 1)
        int endY = startY + 2 - 1; // 7 (length 2)

        // Count corners at target height: must be exactly 1x2 = 2 corners!
        int maxHeightCorners = 0;
        for (int y = 0; y < res.cornerL; ++y) {
            for (int x = 0; x < res.cornerW; ++x) {
                if (res.cornerGrid[y][x] == 1800) {
                    maxHeightCorners++;
                }
            }
        }
        QCOMPARE(maxHeightCorners, 2);

        // Verify plateau corners (distance 0)
        for (int y = startY; y <= endY; ++y) {
            for (int x = startX; x <= endX; ++x) {
                QCOMPARE(res.cornerGrid[y][x], 1800);
            }
        }

        // In Chebyshev distance, ring 1 forms a 3x4 rectangle:
        // [startX - 1, endX + 1] x [startY - 1, endY + 1] -> [5, 7] x [5, 8]
        // In particular, diagonal corner (startX - 1, startY - 1) MUST have height 1500!
        QCOMPARE(res.cornerGrid[startY - 1][startX - 1], 1500);
        QCOMPARE(res.cornerGrid[startY - 1][endX + 1], 1500);
        QCOMPARE(res.cornerGrid[endY + 1][startX - 1], 1500);
        QCOMPARE(res.cornerGrid[endY + 1][endX + 1], 1500);

        // Verify all perimeter corners of ring 1 have height 1500
        for (int y = startY - 1; y <= endY + 1; ++y) {
            for (int x = startX - 1; x <= endX + 1; ++x) {
                if (x < startX || x > endX || y < startY || y > endY) {
                    QCOMPARE(res.cornerGrid[y][x], 1500);
                }
            }
        }

        // Verify diagonal corner of ring 2 is 1200
        QCOMPARE(res.cornerGrid[startY - 2][startX - 2], 1200);

        // Total dirt must be positive and crate count valid
        QVERIFY(res.totalDirt > 0);
        QCOMPARE(res.crates, (res.totalDirt + 299) / 300);
    }

    void testPvPServerSlopeCalculation() {
        // Skill 99.0 on PvP: floor(99.0 * 1.5) = 148 (capped at 150)
        auto resPvp = BridgePillarCalculator::calculate(1, 2, 1800, 99.0, true);
        QCOMPARE(static_cast<int>(resPvp.effectiveSlope), 148);
        // radius = ceil(1800 / 148) = 13
        QCOMPARE(resPvp.spreadRadius, 13);

        // Skill 99.0 on PvE: floor(99.0 * 3.0) = 297 (capped at 300)
        auto resPve = BridgePillarCalculator::calculate(1, 2, 1800, 99.0, false);
        QCOMPARE(static_cast<int>(resPve.effectiveSlope), 297);
        // radius = ceil(1800 / 297) = 7
        QCOMPARE(resPve.spreadRadius, 7);

        // Skill not specified on PvP: capped at 150
        auto resPvpDefault = BridgePillarCalculator::calculate(1, 2, 1800, std::nullopt, true);
        QCOMPARE(static_cast<int>(resPvpDefault.effectiveSlope), 150);
        QCOMPARE(resPvpDefault.spreadRadius, 12);
    }

    void testWidgetPvPToggle() {
        BridgePillarWidget widget;
        
        QCheckBox* pvpCheck = widget.findChild<QCheckBox*>("pvpCheckBox");
        if (!pvpCheck) {
            for (auto* cb : widget.findChildren<QCheckBox*>()) {
                if (cb->text().contains("PvP", Qt::CaseInsensitive)) {
                    pvpCheck = cb;
                    break;
                }
            }
        }
        QVERIFY2(pvpCheck != nullptr, "PvP Server Rules checkbox must exist in BridgePillarWidget");
        QVERIFY(!pvpCheck->isChecked());

        QCheckBox* skillCheck = widget.findChild<QCheckBox*>("skillCheckBox");
        if (!skillCheck) {
            for (auto* cb : widget.findChildren<QCheckBox*>()) {
                if (cb->text().contains("Digging Skill", Qt::CaseInsensitive)) {
                    skillCheck = cb;
                    break;
                }
            }
        }
        QVERIFY2(skillCheck != nullptr, "Limit by Digging Skill checkbox must exist");
        skillCheck->setChecked(true);

        auto* skillSpin = widget.findChild<QDoubleSpinBox*>();
        QVERIFY(skillSpin != nullptr);
        skillSpin->setValue(99.0);

        auto* radiusLabel = widget.findChild<QLabel*>("radiusLabel");
        QVERIFY(radiusLabel != nullptr);
        QVERIFY(radiusLabel->text().contains("Slope: 297"));

        // Toggle PvP on
        pvpCheck->setChecked(true);

        // Verify slope updated dynamically to 148
        QVERIFY(radiusLabel->text().contains("Slope: 148"));
    }

    void testWidgetDefaultPlateau1x2() {
        BridgePillarWidget widget;
        auto* radiusLabel = widget.findChild<QLabel*>("radiusLabel");
        QVERIFY(radiusLabel != nullptr);
        // Default 1x2 input must display Plateau: 1x2, NOT 2x3!
        QVERIFY2(radiusLabel->text().contains("Plateau: 1x2"), 
                 qPrintable(QString("Expected 'Plateau: 1x2' in label, but got: %1").arg(radiusLabel->text())));
        QVERIFY2(!radiusLabel->text().contains("2x3"), "Label should not contain 2x3");
    }
};

QTEST_MAIN(TestBridgePillar)
#include "test_bridge_pillar.moc"
