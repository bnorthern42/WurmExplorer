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
void testBridgePillarPlateauDimensions();
void testBridgePillarRgbColormapAndLoopBounds();

int main() {
    testImpCalculator();
    testBridgePillarCalculator();
    testBridgePillarSlopeAndDigSkillLimit();
    testBridgePillarPlateauDimensions();
    testBridgePillarRgbColormapAndLoopBounds();
    testGrinderEngine();
    testGrinderActionModes();
    testDifficultyPresets();
    std::cout << "\nALL TOOLS TESTS PASSED!" << std::endl;
    return 0;
}

void testBridgePillarPlateauDimensions() {
    std::cout << "Testing BridgePillar plateau tile-to-corner mapping..." << std::endl;

    // Plateau size: 1x2 tiles -> exactly (1+1)x(2+1) = 2x3 corners at target height
    auto res = BridgePillarCalculator::calculate(1, 2, 1800, std::nullopt);
    assert(res.topW == 1);
    assert(res.topL == 2);
    assert(res.plateauCornersX == 2);
    assert(res.plateauCornersY == 3);

    // Count corners at target height in cornerGrid
    int maxHeightCorners = 0;
    int minPlateauX = 9999, maxPlateauX = -1;
    int minPlateauY = 9999, maxPlateauY = -1;
    for (int y = 0; y < res.cornerL; ++y) {
        for (int x = 0; x < res.cornerW; ++x) {
            if (res.cornerGrid[y][x] == 1800) {
                maxHeightCorners++;
                minPlateauX = std::min(minPlateauX, x);
                maxPlateauX = std::max(maxPlateauX, x);
                minPlateauY = std::min(minPlateauY, y);
                maxPlateauY = std::max(maxPlateauY, y);
            }
        }
    }
    // Must be exactly 2 columns and 3 rows = 6 corners
    assert(maxHeightCorners == 6);
    assert((maxPlateauX - minPlateauX + 1) == 2);
    assert((maxPlateauY - minPlateauY + 1) == 3);

    // Footprint dimensions: baseCorners = plateauCorners + 2 * r
    // r = 6 -> baseCornersX = 2 + 12 = 14, baseCornersY = 3 + 12 = 15
    assert(res.cornerW == 14);
    assert(res.cornerL == 15);
    assert(res.baseW == 13);
    assert(res.baseL == 14);

    std::cout << "BridgePillar plateau dimensions tests passed!" << std::endl;
}

void testDifficultyPresets() {
    std::cout << "Testing DifficultyPresets provider with real Wurm data..." << std::endl;

    // Test Mining presets exist and contain exact Google Sheet difficulties
    // MiningPower: Stone 2.0, Iron 3.0, Sandstone 45.0, Marble 40.0, Adamantine 60.0
    auto miningPower = DifficultyProvider::getPresetsForMode(ActionMode::MiningPower);
    assert(!miningPower.empty());
    bool foundIron = false;
    bool foundSandstone = false;
    bool foundMarble = false;
    bool foundAdamantine = false;
    for (const auto& [name, diff] : miningPower) {
        if (name.contains("Iron") && approxEqual(diff, 3.0)) foundIron = true;
        if (name.contains("Sandstone") && approxEqual(diff, 45.0)) foundSandstone = true;
        if (name.contains("Marble") && approxEqual(diff, 40.0)) foundMarble = true;
        if (name.contains("Adamantine") && approxEqual(diff, 60.0)) foundAdamantine = true;
    }
    assert(foundIron);
    assert(foundSandstone);
    assert(foundMarble);
    assert(foundAdamantine);

    // Test MiningQl presets also contain exact values
    auto miningQl = DifficultyProvider::getPresetsForMode(ActionMode::MiningQl);
    assert(!miningQl.empty());
    bool foundGlimmersteel = false;
    for (const auto& [name, diff] : miningQl) {
        if (name.contains("Glimmersteel") && approxEqual(diff, 55.0)) foundGlimmersteel = true;
    }
    assert(foundGlimmersteel);

    // Test Taming presets exist and contain exact Google Sheet data & aged variants
    auto tamingPresets = DifficultyProvider::getPresetsForMode(ActionMode::Taming);
    assert(!tamingPresets.empty());
    bool foundCow = false;
    bool foundHorse = false;
    bool foundHellHorse = false;
    bool foundHorseYoung = false;
    bool foundHorseMature = false;
    for (const auto& [name, diff] : tamingPresets) {
        if (name.contains("Cow") && approxEqual(diff, 10.0)) foundCow = true;
        if (name == "Horse" && approxEqual(diff, 132.0)) foundHorse = true;
        if (name.contains("Hell Horse") && (approxEqual(diff, 648.0) || approxEqual(diff, 712.8))) foundHellHorse = true;
        if (name.contains("Horse (Young)") && approxEqual(diff, 118.8)) foundHorseYoung = true;
        if (name.contains("Horse (Mature)") && approxEqual(diff, 145.2)) foundHorseMature = true;
    }
    assert(foundCow);
    assert(foundHorse);
    assert(foundHellHorse);
    assert(foundHorseYoung);
    assert(foundHorseMature);

    // Test all 14 action modes return non-empty presets
    for (int m = 0; m <= static_cast<int>(ActionMode::Shearing); ++m) {
        auto mode = static_cast<ActionMode>(m);
        auto p = DifficultyProvider::getPresetsForMode(mode);
        assert(!p.empty());
    }

    std::cout << "DifficultyPresets tests passed!" << std::endl;
}

void testBridgePillarRgbColormapAndLoopBounds() {
    std::cout << "Testing BridgePillar RGB colormap and loop bounds..." << std::endl;

    // 1. Test elevation tiers for every 300 dirt (Wurm slope limits)
    assert(BridgePillarCalculator::getElevationTier(0) == 0);
    assert(BridgePillarCalculator::getElevationTier(150) == 1);
    assert(BridgePillarCalculator::getElevationTier(300) == 1);
    assert(BridgePillarCalculator::getElevationTier(301) == 2);
    assert(BridgePillarCalculator::getElevationTier(600) == 2);
    assert(BridgePillarCalculator::getElevationTier(900) == 3);
    assert(BridgePillarCalculator::getElevationTier(1200) == 4);
    assert(BridgePillarCalculator::getElevationTier(1500) == 5);
    assert(BridgePillarCalculator::getElevationTier(1800) == 6);

    // 2. Test RGB color styling for every 300 on the border of each
    auto style0 = BridgePillarCalculator::getElevationStyle(0);
    assert(style0.tier == 0);

    auto style300 = BridgePillarCalculator::getElevationStyle(300);
    assert(style300.tier == 1);
    assert(style300.border.b > style300.border.r && style300.border.b > style300.border.g);

    auto style600 = BridgePillarCalculator::getElevationStyle(600);
    assert(style600.tier == 2);
    assert(style600.border.g > 150 && style600.border.b > 150 && style600.border.r < 80);

    auto style900 = BridgePillarCalculator::getElevationStyle(900);
    assert(style900.tier == 3);
    assert(style900.border.g > style900.border.r && style900.border.g > style900.border.b);

    auto style1200 = BridgePillarCalculator::getElevationStyle(1200);
    assert(style1200.tier == 4);
    assert(style1200.border.r > 200 && style1200.border.g > 120 && style1200.border.b < 80);

    auto style1500 = BridgePillarCalculator::getElevationStyle(1500);
    assert(style1500.tier == 5);
    assert(style1500.border.r > 200 && style1500.border.g > 80 && style1500.border.b < 80);

    auto style1800 = BridgePillarCalculator::getElevationStyle(1800);
    assert(style1800.tier == 6);
    assert(style1800.border.r > 200 && style1800.border.g < 100 && style1800.border.b < 100);

    // 3. Test loop bounds: 2x2 tile plateau must produce exactly 3x3 max-height corners
    auto res2x2 = BridgePillarCalculator::calculate(2, 2, 1800, std::nullopt);
    assert(res2x2.plateauCornersX == 3);
    assert(res2x2.plateauCornersY == 3);
    int count1800 = 0;
    for (int y = 0; y < res2x2.cornerL; ++y) {
        for (int x = 0; x < res2x2.cornerW; ++x) {
            if (res2x2.cornerGrid[y][x] == 1800) count1800++;
        }
    }
    assert(count1800 == 9);

    std::cout << "BridgePillar RGB colormap and loop bounds tests passed!" << std::endl;
}

