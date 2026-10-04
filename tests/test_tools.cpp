#include <cassert>
#include <iostream>
#include <cmath>
#include "../src/features/tools/ImpCalculator.hpp"
#include "../src/features/tools/BridgePillarCalculator.hpp"
#include "../src/features/tools/GrinderEngine.hpp"
#include "../src/features/tools/DifficultyPresets.hpp"

using namespace tools;

static bool approxEqual(double a, double b, double eps = 0.05) {
    return std::fabs(a - b) < eps;
}

void testImpCalculator() {
    std::cout << "Testing ImpCalculator..." << std::endl;

    // Test max impable QL: (skill * 0.77) + 23
    assert(approxEqual(ImpCalculator::calculateMaxImpQl(0.0), 23.0));
    assert(approxEqual(ImpCalculator::calculateMaxImpQl(50.0), 61.5));
    assert(approxEqual(ImpCalculator::calculateMaxImpQl(100.0), 100.0));

    // Test skill required for desired QL: (targetQl - 23) / 0.77
    assert(approxEqual(ImpCalculator::calculateSkillNeeded(61.5), 50.0));
    assert(approxEqual(ImpCalculator::calculateSkillNeeded(100.0), 100.0));
    assert(ImpCalculator::calculateSkillNeeded(20.0) <= 0.0); // Less than 23.77 returns <= 0

    std::cout << "ImpCalculator tests passed!" << std::endl;
}

void testBridgePillarCalculator() {
    std::cout << "Testing BridgePillarCalculator..." << std::endl;

    // Standard test: 1x2 plateau, height 1800, max slope 300 (no dig skill specified)
    auto res1 = BridgePillarCalculator::calculate(1, 2, 1800, std::nullopt);
    assert(res1.topW == 1);
    assert(res1.topL == 2);
    assert(res1.targetHeight == 1800);
    assert(approxEqual(res1.effectiveSlope, 300.0));
    assert(res1.spreadRadius == 6); // ceil(1800 / 300) = 6
    assert(res1.baseW == 1 + 2 * 6); // 13
    assert(res1.baseL == 2 + 2 * 6); // 14
    assert(res1.cornerW == 14);
    assert(res1.cornerL == 15);
    assert(res1.totalDirt > 0);
    assert(res1.crates == (res1.totalDirt + 299) / 300);

    // Verify plateau corners are at target height
    assert(res1.cornerGrid.size() == 15);
    assert(res1.cornerGrid[0].size() == 14);
    assert(res1.cornerGrid[6][6] == 1800);
    assert(res1.cornerGrid[8][7] == 1800);
    assert(res1.cornerGrid[0][0] == 0); // Far corner reaches 0

    // Verify tile grid dimensions and plateau
    assert(res1.tileGrid.size() == 14);
    assert(res1.tileGrid[0].size() == 13);
    assert(res1.tileGrid[6][6] == 1800);

    // Test with digging skill limiting slope: skill 50 -> slope min(300, 3*50) = 150
    auto res2 = BridgePillarCalculator::calculate(2, 2, 1800, 50.0);
    assert(approxEqual(res2.effectiveSlope, 150.0));
    assert(res2.spreadRadius == 12); // ceil(1800 / 150) = 12
    assert(res2.baseW == 2 + 2 * 12); // 26
    assert(res2.baseL == 2 + 2 * 12); // 26

    std::cout << "BridgePillarCalculator tests passed!" << std::endl;
}

void testGrinderEngine() {
    std::cout << "Testing GrinderEngine..." << std::endl;

    // Test effectiveSkill formula
    assert(approxEqual(GrinderEngine::effectiveSkill(50.0, 0.0), 50.0));
    assert(approxEqual(GrinderEngine::effectiveSkill(50.0, 20.0), 55.0)); // diffToMax = 25; 25 * 0.20 = 5
    assert(approxEqual(GrinderEngine::effectiveSkill(50.0, 100.0), GrinderEngine::effectiveSkill(50.0, 70.0))); // Capped at 70

    // Test effectiveWithItem formula
    assert(approxEqual(GrinderEngine::effectiveWithItem(50.0, 50.0, 0.0), 50.0));
    assert(approxEqual(GrinderEngine::effectiveWithItem(50.0, 20.0, 0.0), 35.0)); // ql < skill: (50 + 20) / 2
    assert(approxEqual(GrinderEngine::effectiveWithItem(50.0, 70.0, 0.0), 60.0)); // ql > skill: 50 + 50 * 20 / 100

    // Test roll bounds and Monte Carlo distribution
    auto highCheck = GrinderEngine::simulateSkillCheck(90.0, 15.0, 80.0, 10.0, 2000, 42);
    assert(highCheck.totalTrials == 2000);
    assert(highCheck.successRate > 85.0); // High skill should easily succeed (> 85%)
    assert(highCheck.min >= -100.0 && highCheck.max <= 100.0);

    auto lowCheck = GrinderEngine::simulateSkillCheck(15.0, 90.0, 20.0, 0.0, 2000, 42);
    assert(lowCheck.successRate < 15.0); // Low skill should fail most of the time (< 15%)

    // Test mining simulation produces valid ore quality within bounds
    auto miningRes = GrinderEngine::simulateMiningQl(50.0, 20.0, 50.0, 50.0, 0.0, 50.0, 0, 0.0, 1000, 42);
    assert(miningRes.mean >= 20.0 && miningRes.mean <= 100.0);
    assert(miningRes.min >= 1.0);

    std::cout << "GrinderEngine tests passed!" << std::endl;
}

void testGrinderActionModes() {
    std::cout << "Testing GrinderEngine all action modes..." << std::endl;

    // 1. GenericCheck
    GrinderParams pGeneric;
    pGeneric.mode = ActionMode::GenericCheck;
    pGeneric.skill = 60.0;
    pGeneric.difficulty = 20.0;
    pGeneric.itemQl = 50.0;
    auto resGen = GrinderEngine::simulate(pGeneric, 1000, 42);
    assert(resGen.totalTrials == 1000);
    assert(resGen.successRate > 75.0);

    // 2. MiningPower
    GrinderParams pMinePow;
    pMinePow.mode = ActionMode::MiningPower;
    pMinePow.skill = 60.0;
    pMinePow.difficulty = 20.0;
    pMinePow.secondaryQl = 70.0;
    pMinePow.secondarySkill = 50.0;
    auto resMinePow = GrinderEngine::simulate(pMinePow, 1000, 42);
    assert(resMinePow.totalTrials == 1000);
    assert(resMinePow.successRate > 75.0);

    // 3. MiningQl
    GrinderParams pMineQl;
    pMineQl.mode = ActionMode::MiningQl;
    pMineQl.skill = 50.0;
    pMineQl.difficulty = 20.0;
    pMineQl.secondaryQl = 50.0;
    pMineQl.secondarySkill = 50.0;
    pMineQl.veinQl = 60.0;
    auto resMineQl = GrinderEngine::simulate(pMineQl, 1000, 42);
    assert(resMineQl.totalTrials == 1000);
    assert(resMineQl.mean >= 20.0 && resMineQl.mean <= 100.0);

    // 4. Farming
    GrinderParams pFarm;
    pFarm.mode = ActionMode::Farming;
    pFarm.skill = 70.0;
    pFarm.difficulty = 20.0;
    pFarm.secondaryQl = 60.0;
    pFarm.secondarySkill = 50.0;
    pFarm.tertiarySkill = 40.0;
    auto resFarm = GrinderEngine::simulate(pFarm, 1000, 42);
    assert(resFarm.totalTrials == 1000);
    assert(resFarm.successRate > 80.0);

    // 5. Digging (slope test)
    GrinderParams pDigFlat;
    pDigFlat.mode = ActionMode::Digging;
    pDigFlat.skill = 50.0;
    pDigFlat.difficulty = 10.0;
    pDigFlat.secondaryQl = 50.0;
    pDigFlat.slope = 0.0;
    auto resDigFlat = GrinderEngine::simulate(pDigFlat, 1000, 42);

    GrinderParams pDigSteep = pDigFlat;
    pDigSteep.slope = 100.0;
    auto resDigSteep = GrinderEngine::simulate(pDigSteep, 1000, 42);
    assert(resDigFlat.mean > resDigSteep.mean);

    // 6. Meditation
    GrinderParams pMed;
    pMed.mode = ActionMode::Meditation;
    pMed.skill = 60.0;
    pMed.secondaryQl = 50.0;
    pMed.pathLevel = 5;
    pMed.mediTile = true;
    pMed.mediCooldown = false;
    auto resMed = GrinderEngine::simulate(pMed, 1000, 42);
    assert(resMed.totalTrials == 1000);

    // 7. Creation (material QL cap test)
    GrinderParams pCreate;
    pCreate.mode = ActionMode::Creation;
    pCreate.skill = 80.0;
    pCreate.difficulty = 10.0;
    pCreate.secondaryQl = 80.0;
    pCreate.secondarySkill = 70.0;
    pCreate.materialQl = 35.0;
    auto resCreate = GrinderEngine::simulate(pCreate, 1000, 42);
    assert(resCreate.totalTrials == 1000);
    assert(resCreate.max <= 35.05);

    // 8. WoodcuttingQl
    GrinderParams pWood;
    pWood.mode = ActionMode::WoodcuttingQl;
    pWood.skill = 50.0;
    pWood.difficulty = 20.0;
    pWood.secondaryQl = 50.0;
    pWood.secondarySkill = 50.0;
    auto resWood = GrinderEngine::simulate(pWood, 1000, 42);
    assert(resWood.totalTrials == 1000);
    assert(resWood.min >= 1.0);

    // 9. Imping
    GrinderParams pImp;
    pImp.mode = ActionMode::Imping;
    pImp.skill = 70.0;
    pImp.targetQl = 40.0;
    pImp.secondaryQl = 60.0;
    pImp.secondarySkill = 50.0;
    auto resImp = GrinderEngine::simulate(pImp, 1000, 42);
    assert(resImp.totalTrials == 1000);
    assert(resImp.successRate > 60.0);

    // 10. SmithingSteps
    GrinderParams pSmith;
    pSmith.mode = ActionMode::SmithingSteps;
    pSmith.skill = 60.0;
    pSmith.startQl = 30.0;
    pSmith.targetQl = 50.0;
    pSmith.secondaryQl = 60.0;
    auto resSmith = GrinderEngine::simulate(pSmith, 200, 42);
    assert(resSmith.totalTrials == 200);
    assert(resSmith.mean > 0.0);

    // 11. Taming (compare easy vs hard mob)
    GrinderParams pTameEasy;
    pTameEasy.mode = ActionMode::Taming;
    pTameEasy.skill = 60.0;
    pTameEasy.secondarySkill = 50.0;
    pTameEasy.mobName = "Chicken";
    pTameEasy.mobSstr = 5.0;
    pTameEasy.mobCr = 1.0;
    auto resTameEasy = GrinderEngine::simulate(pTameEasy, 1000, 42);

    GrinderParams pTameHard = pTameEasy;
    pTameHard.mobName = "DragonGreen";
    pTameHard.mobSstr = 60.0;
    pTameHard.mobCr = 100.0;
    auto resTameHard = GrinderEngine::simulate(pTameHard, 1000, 42);
    assert(resTameEasy.successRate > resTameHard.successRate);

    // 12. Fileting
    GrinderParams pFilet;
    pFilet.mode = ActionMode::Fileting;
    pFilet.skill = 60.0;
    pFilet.difficulty = 20.0;
    pFilet.secondaryQl = 50.0;
    pFilet.secondarySkill = 50.0;
    pFilet.tertiarySkill = 40.0;
    auto resFilet = GrinderEngine::simulate(pFilet, 1000, 42);
    assert(resFilet.totalTrials == 1000);

    // 13. Forestry
    GrinderParams pForest;
    pForest.mode = ActionMode::Forestry;
    pForest.skill = 60.0;
    pForest.secondaryQl = 50.0;
    pForest.secondarySkill = 50.0;
    auto resForest = GrinderEngine::simulate(pForest, 1000, 42);
    assert(resForest.totalTrials == 1000);

    // 14. Shearing (young vs adult sheep)
    GrinderParams pShearYoung;
    pShearYoung.mode = ActionMode::Shearing;
    pShearYoung.skill = 50.0;
    pShearYoung.secondaryQl = 50.0;
    pShearYoung.sheepAge = 2;
    auto resShearYoung = GrinderEngine::simulate(pShearYoung, 1000, 42);

    GrinderParams pShearOld = pShearYoung;
    pShearOld.sheepAge = 50;
    auto resShearOld = GrinderEngine::simulate(pShearOld, 1000, 42);
    assert(resShearYoung.successRate > resShearOld.successRate);

    std::cout << "All 14 GrinderEngine action modes tested successfully!" << std::endl;
}

void testBridgePillarSlopeAndDigSkillLimit() {
    std::cout << "Testing BridgePillar slope logic and digging skill limit..." << std::endl;

    // Test with digging skill 20.0:
    // Max Slope must strictly adhere to: floor(DiggingSkill * 3) = floor(20.0 * 3) = 60
    auto resSkill20 = BridgePillarCalculator::calculate(1, 2, 1800, 20.0);
    assert(resSkill20.effectiveSlope == 60.0);
    assert(resSkill20.spreadRadius == 30); // 1800 / 60 = 30
    assert(resSkill20.baseW == 1 + 2 * 30); // 61
    assert(resSkill20.baseL == 2 + 2 * 30); // 62
    assert(resSkill20.cornerW == 62);
    assert(resSkill20.cornerL == 63);

    // Test with fractional digging skill 20.7:
    // floor(20.7 * 3) = floor(62.1) = 62 (NOT 62.1!)
    auto resSkillFract = BridgePillarCalculator::calculate(1, 2, 1800, 20.7);
    assert(resSkillFract.effectiveSlope == 62.0); // Strict floor check!
    assert(resSkillFract.spreadRadius == (1800 + 61) / 62); // ceil(1800 / 62) = 30

    // Test drop-off step-down per tile from plateau:
    // 1 tile away along X: height must drop by exactly effectiveSlope (62)
    int plateauX = resSkillFract.spreadRadius;
    int plateauY = resSkillFract.spreadRadius;
    assert(resSkillFract.cornerGrid[plateauY][plateauX] == 1800);
    assert(resSkillFract.cornerGrid[plateauY][plateauX - 1] == 1800 - 62);
    assert(resSkillFract.cornerGrid[plateauY][plateauX - 2] == 1800 - 62 * 2);

    // Test cap at 300: skill 100 -> 3 * 100 = 300
    auto resSkill100 = BridgePillarCalculator::calculate(1, 2, 1800, 100.0);
    assert(resSkill100.effectiveSlope == 300.0);
    assert(resSkill100.spreadRadius == 6);

    std::cout << "BridgePillar slope tests passed!" << std::endl;
}

void testDifficultyPresets();

int main() {
    testImpCalculator();
    testBridgePillarCalculator();
    testBridgePillarSlopeAndDigSkillLimit();
    testGrinderEngine();
    testGrinderActionModes();
    testDifficultyPresets();
    std::cout << "\nALL TOOLS TESTS PASSED!" << std::endl;
    return 0;
}

void testDifficultyPresets() {
    std::cout << "Testing DifficultyPresets provider..." << std::endl;

    // Test Mining presets exist and contain Iron Vein (20)
    auto miningPresets = DifficultyProvider::getPresetsForMode(ActionMode::MiningPower);
    assert(!miningPresets.empty());
    bool foundIron = false;
    for (const auto& [name, diff] : miningPresets) {
        if (name.contains("Iron") && approxEqual(diff, 20.0)) {
            foundIron = true;
            break;
        }
    }
    assert(foundIron);

    // Test Taming presets exist and contain Cow (20) and Hell Horse (80)
    auto tamingPresets = DifficultyProvider::getPresetsForMode(ActionMode::Taming);
    assert(!tamingPresets.empty());
    bool foundCow = false;
    bool foundHellHorse = false;
    for (const auto& [name, diff] : tamingPresets) {
        if (name.contains("Cow") && approxEqual(diff, 20.0)) foundCow = true;
        if (name.contains("Hell Horse") && approxEqual(diff, 80.0)) foundHellHorse = true;
    }
    assert(foundCow);
    assert(foundHellHorse);

    // Test all 14 action modes return non-empty presets
    for (int m = 0; m <= static_cast<int>(ActionMode::Shearing); ++m) {
        auto mode = static_cast<ActionMode>(m);
        auto p = DifficultyProvider::getPresetsForMode(mode);
        assert(!p.empty());
    }

    std::cout << "DifficultyPresets tests passed!" << std::endl;
}
