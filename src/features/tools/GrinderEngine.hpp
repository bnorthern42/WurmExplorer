#pragma once

#include <vector>
#include <map>
#include <string>
#include <random>

namespace tools {

enum class ActionMode {
    GenericCheck = 0,
    MiningPower,
    MiningQl,
    Farming,
    Digging,
    Meditation,
    Creation,
    WoodcuttingQl,
    Imping,
    SmithingSteps,
    Taming,
    Fileting,
    Forestry,
    Shearing
};

struct GrinderParams {
    ActionMode mode = ActionMode::GenericCheck;
    double skill = 50.0;
    double difficulty = 20.0;
    double itemQl = 50.0;
    double bonus = 0.0;
    bool benediction = false;

    // Secondary skills & tools
    double secondarySkill = 50.0;  // pick_skill / rake_skill / hatchet_skill / tool_skill / soul / knife_skill / sickle_skill
    double secondaryQl = 50.0;     // pick_ql / rake_ql / hatchet_ql / tool_ql / shovel_ql / rug_ql / knife_ql / scissor_ql
    double tertiarySkill = 50.0;   // nature_skill / parent_skill / knives_skill / cooking_skill

    // Mode-specific modifiers
    double veinQl = 50.0;
    int imbue = 0;
    double rarity = 0.0;
    double rune = 0.0;
    double slope = 0.0;            // digging slope
    int pathLevel = 5;             // meditation path level
    bool mediTile = true;
    bool mediCooldown = false;
    double materialQl = 50.0;      // creation material QL
    double startQl = 30.0;         // smithing start QL
    double targetQl = 70.0;        // smithing / imping target QL
    std::string mobName = "Sheep"; // taming mob name
    double mobSstr = 10.0;         // taming mob sstr
    double mobCr = 1.0;            // taming mob CR
    double tamingModifier = 1.0;
    double tamingAge = 1.0;
    bool isFo = false;
    bool isHots = false;
    bool isTamed = false;
    int sheepAge = 15;             // shearing age
};

struct RollDistribution {
    double successRate = 0.0; // percentage [0.0 - 100.0]
    double mean = 0.0;
    double min = 0.0;
    double max = 0.0;
    std::map<int, int> histogram; // power/ql bucket -> count
    int totalTrials = 0;
};

class GrinderEngine {
public:
    static double standardNormal(std::mt19937_64& rng);
    static double effectiveSkill(double skill, double bonus);
    static double effectiveWithItem(double skill, double ql, double bonus);
    static double rollGaussian(double skill, double difficulty, std::mt19937_64& rng);
    static double skillCheck(double skill, double difficulty, double ql, double bonus, std::mt19937_64& rng);
    static double calcRareQuality(double power, double bonus);
    static double calcOreRareQuality(double power, double bonus);

    // Monte Carlo Simulations
    static RollDistribution simulate(const GrinderParams& params, int trials = 5000, uint64_t seed = 0);
    static RollDistribution simulateSkillCheck(double skill, double difficulty, double ql, double bonus, int trials = 5000, uint64_t seed = 0);
    static RollDistribution simulateMiningQl(double skill, double difficulty, double pickQl, double pickSkill, double pickRarity, double veinQl, int imbue, double rune, int trials = 5000, uint64_t seed = 0);
    static RollDistribution simulateSmithingSteps(double skill, double startQl, double targetQl, double toolQl, int imbue, int trials = 1000, uint64_t seed = 0);
};

} // namespace tools
