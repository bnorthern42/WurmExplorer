#include "GrinderEngine.hpp"
#include "GrinderMobData.hpp"
#include <algorithm>
#include <cmath>

namespace tools {

static double rareQuality(double power, double bonus, double numbBonus, double fiddle) {
    if (bonus > 0.0) {
        double val = fiddle - power;
        double square = val * val;
        double n = square / 1000.0;
        double mod = std::min(n * 1.25, 1.0);
        bonus = bonus * 3.0 / numbBonus * mod;
    }
    return std::max(1.0, std::min(99.999, power + bonus));
}

double GrinderEngine::calcRareQuality(double power, double bonus) {
    return rareQuality(power, bonus, 3.0, 100.0);
}

double GrinderEngine::calcOreRareQuality(double power, double bonus) {
    return rareQuality(power, bonus, 2.0, 108.428);
}

double GrinderEngine::standardNormal(std::mt19937_64& rng) {
    std::normal_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

double GrinderEngine::effectiveSkill(double skill, double bonus) {
    if (bonus > 70.0) bonus = 70.0;
    if (bonus <= 0.0) return skill;
    double linearMax = (100.0 + skill) / 2.0;
    double diffToMaxChange = std::min(skill, linearMax - skill);
    double newBon = diffToMaxChange * bonus / 100.0;
    return skill + newBon;
}

double GrinderEngine::effectiveWithItem(double skill, double ql, double bonus) {
    if (bonus > 70.0) bonus = 70.0;
    double bonusSkill = 0.0;
    if (ql < skill) {
        bonusSkill = (skill + ql) / 2.0;
    } else {
        double linearMax = ql - skill;
        bonusSkill = skill + skill * linearMax / 100.0;
    }

    if (bonus > 0.0) {
        double linearMax = (100.0 + bonusSkill) / 2.0;
        double diffToMaxChange = std::min(bonusSkill, linearMax - bonusSkill);
        double newBon = diffToMaxChange * bonus / 100.0;
        bonusSkill += newBon;
    }
    return bonusSkill;
}

double GrinderEngine::rollGaussian(double skill, double difficulty, std::mt19937_64& rng) {
    double slide = (std::pow(skill, 3) - std::pow(difficulty, 3)) / 50000.0 + (skill - difficulty);
    double w = 30.0 - std::abs(skill - difficulty) / 4.0;

    int attempts = 0;
    double result = 0.0;
    std::uniform_real_distribution<double> uniform01(0.0, 1.0);

    while (true) {
        result = standardNormal(rng) * (w + std::abs(slide) / 6.0) + slide;
        double rejectCutoff = standardNormal(rng) * (w - std::abs(slide) / 6.0) + slide;
        if (slide > 0.0) {
            if (result > rejectCutoff + std::max(100.0 - slide, 0.0)) {
                result = -1000.0;
            }
        } else if (result < rejectCutoff - std::max(100.0 + slide, 0.0)) {
            result = -1000.0;
        }
        attempts++;
        if (attempts >= 100) {
            if (result > 100.0) {
                return 90.0 + uniform01(rng) * 5.0;
            }
            if (result < -100.0) {
                return -90.0 - uniform01(rng) * 5.0;
            }
        }

        if (result > -100.0 && result < 100.0) {
            break;
        }
    }
    return result;
}

double GrinderEngine::skillCheck(double skill, double difficulty, double ql, double bonus, std::mt19937_64& rng) {
    double effective = (ql <= 0.0) ? effectiveSkill(skill, bonus) : effectiveWithItem(skill, ql, bonus);
    return rollGaussian(effective, difficulty, rng);
}

RollDistribution GrinderEngine::simulateSkillCheck(double skill, double difficulty, double ql, double bonus, int trials, uint64_t seed) {
    std::mt19937_64 rng(seed != 0 ? seed : 1337);
    RollDistribution dist;
    dist.totalTrials = trials;
    if (trials <= 0) return dist;

    int successes = 0;
    double sum = 0.0;
    dist.min = 1e9;
    dist.max = -1e9;

    for (int i = 0; i < trials; ++i) {
        double roll = skillCheck(skill, difficulty, ql, bonus, rng);
        if (roll > 0.0) successes++;
        sum += roll;
        if (roll < dist.min) dist.min = roll;
        if (roll > dist.max) dist.max = roll;
        int bucket = static_cast<int>(std::round(roll));
        dist.histogram[bucket]++;
    }

    dist.successRate = (static_cast<double>(successes) / trials) * 100.0;
    dist.mean = sum / trials;
    return dist;
}

RollDistribution GrinderEngine::simulateMiningQl(double skill, double difficulty, double pickQl, double pickSkill, double pickRarity, double veinQl, int imbue, double rune, int trials, uint64_t seed) {
    std::mt19937_64 rng(seed != 0 ? seed : 1337);
    RollDistribution dist;
    dist.totalTrials = trials;
    if (trials <= 0) return dist;

    int successes = 0;
    double sum = 0.0;
    dist.min = 1e9;
    dist.max = -1e9;
    double imbueEnhancement = 1.0 + 0.23047 * static_cast<double>(imbue) / 100.0;

    for (int i = 0; i < trials; ++i) {
        double pickBonus = skillCheck(pickSkill, difficulty, pickQl, 0.0, rng) / 5.0;
        double miningPower = skillCheck(skill, difficulty, pickQl, pickBonus, rng);
        double power = std::max(1.0, miningPower);
        if (skill * imbueEnhancement < power) {
            power = skill * imbueEnhancement;
        }

        double maxOre = std::min(100.0, 20.0 + veinQl * imbueEnhancement + pickRarity);
        power = std::min(power, maxOre);
        double orePower = calcOreRareQuality(power * (1.0 + rune), pickRarity);

        if (miningPower > 0.0) successes++;
        sum += orePower;
        if (orePower < dist.min) dist.min = orePower;
        if (orePower > dist.max) dist.max = orePower;
        int bucket = static_cast<int>(std::round(orePower));
        dist.histogram[bucket]++;
    }

    dist.successRate = (static_cast<double>(successes) / trials) * 100.0;
    dist.mean = sum / trials;
    return dist;
}

RollDistribution GrinderEngine::simulateSmithingSteps(double skill, double startQl, double targetQl, double toolQl, int imbue, int trials, uint64_t seed) {
    std::mt19937_64 rng(seed != 0 ? seed : 1337);
    std::uniform_real_distribution<double> u01(0.0, 1.0);
    RollDistribution dist;
    dist.totalTrials = trials;
    if (trials <= 0) return dist;

    double sumSteps = 0.0;
    dist.min = 1e9;
    dist.max = -1e9;

    double imbueEnhancement = 1.0 + static_cast<double>(imbue) / 100.0;
    double improveBonus = 0.23047 * imbueEnhancement;
    double maxPossible = skill + (100.0 - skill) * improveBonus;

    for (int t = 0; t < trials; ++t) {
        int steps = 0;
        double currentQl = startQl;
        double highestQl = startQl;

        while (currentQl < targetQl && steps < 500) {
            steps++;
            double diff = std::max(0.0, maxPossible - currentQl);
            double power = skillCheck(skill, currentQl, toolQl, 0.0, rng);
            if (power < 0.0) continue;

            double mod = (100.0 - currentQl) / 2000.0 * (u01(rng) + u01(rng) + u01(rng) + u01(rng)) / 2.0;
            if (diff <= 0.0) mod *= 0.01;
            if (currentQl < highestQl) mod *= 2.0;

            double actionPower = mod * std::max(1.0, diff);
            currentQl = std::min(100.0, currentQl + actionPower);
            if (currentQl > highestQl) highestQl = currentQl;
        }

        sumSteps += steps;
        if (steps < dist.min) dist.min = steps;
        if (steps > dist.max) dist.max = steps;
        dist.histogram[steps]++;
    }

    dist.successRate = 100.0;
    dist.mean = sumSteps / trials;
    return dist;
}

RollDistribution GrinderEngine::simulate(const GrinderParams& params, int trials, uint64_t seed) {
    if (params.mode == ActionMode::SmithingSteps) {
        return simulateSmithingSteps(params.skill, params.startQl, params.targetQl, params.secondaryQl, params.imbue, trials, seed);
    }

    if (trials <= 0) trials = 5000;
    std::mt19937_64 rng(seed != 0 ? seed : 1337);
    std::uniform_real_distribution<double> u01(0.0, 1.0);
    RollDistribution dist;
    dist.totalTrials = trials;

    int successes = 0;
    double sum = 0.0;
    dist.min = 1e9;
    dist.max = -1e9;

    double mobSstr = params.mobSstr;
    double mobCr = params.mobCr;
    if (params.mode == ActionMode::Taming) {
        const auto& mobs = getTamingMobs();
        auto it = mobs.find(params.mobName);
        if (it != mobs.end()) {
            mobSstr = it->second.sstr;
            mobCr = it->second.cr;
        }
    }

    for (int t = 0; t < trials; ++t) {
        double rollVal = 0.0;
        bool isSuccess = false;

        switch (params.mode) {
        case ActionMode::GenericCheck: {
            rollVal = skillCheck(params.skill, params.difficulty, params.itemQl, params.bonus, rng);
            if (params.benediction) rollVal += 5.0;
            isSuccess = (rollVal > 0.0);
            break;
        }
        case ActionMode::MiningPower: {
            double pickBonus = skillCheck(params.secondarySkill, params.difficulty, params.secondaryQl, 0.0, rng) / 5.0;
            rollVal = skillCheck(params.skill, params.difficulty, params.secondaryQl, pickBonus, rng);
            isSuccess = (rollVal > 0.0);
            break;
        }
        case ActionMode::MiningQl: {
            double pickBonus = skillCheck(params.secondarySkill, params.difficulty, params.secondaryQl, 0.0, rng) / 5.0;
            double miningPower = skillCheck(params.skill, params.difficulty, params.secondaryQl, pickBonus, rng);
            double power = std::max(1.0, miningPower);
            double imbueEnhancement = 1.0 + 0.23047 * static_cast<double>(params.imbue) / 100.0;
            if (params.skill * imbueEnhancement < power) {
                power = params.skill * imbueEnhancement;
            }
            double maxOre = std::min(100.0, 20.0 + params.veinQl * imbueEnhancement + params.rarity);
            power = std::min(power, maxOre);
            rollVal = calcOreRareQuality(power * (1.0 + params.rune), params.rarity);
            isSuccess = (miningPower > 0.0);
            break;
        }
        case ActionMode::Farming: {
            double rakeBonus = skillCheck(params.secondarySkill, params.difficulty, params.secondaryQl, 0.0, rng);
            double natureBonus = rollGaussian(params.tertiarySkill, params.difficulty, rng) / 10.0;
            double bonus = rakeBonus + std::max(0.0, natureBonus);
            rollVal = skillCheck(params.skill, params.difficulty, params.secondaryQl, bonus, rng);
            isSuccess = (rollVal > 0.0);
            break;
        }
        case ActionMode::Digging: {
            double effDiff = params.difficulty + 1.0 + params.slope / 5.0;
            double effDig = effectiveWithItem(params.skill, params.secondaryQl, 0.0);
            rollVal = rollGaussian(effDig, effDiff, rng);
            isSuccess = (rollVal > 0.0);
            break;
        }
        case ActionMode::Meditation: {
            double diff = 5.0;
            if (params.mediTile) {
                diff = 10.0;
                if (!params.mediCooldown) {
                    if (params.pathLevel * 10.0 - params.skill < 30.0 || params.skill > 90.0) {
                        diff = 1.0 + params.pathLevel * 10.0;
                    } else {
                        diff = 1.0 + params.pathLevel * 3.0;
                    }
                }
            }
            double natureBonus = std::max(0.0, rollGaussian(params.tertiarySkill, diff, rng) / 10.0);
            rollVal = skillCheck(params.skill, diff, params.secondaryQl, natureBonus, rng);
            isSuccess = (rollVal > 0.0);
            break;
        }
        case ActionMode::Creation: {
            double bonus = 0.0;
            if (params.secondarySkill > 0.0) {
                bonus += std::max(-10.0, rollGaussian(params.secondarySkill, params.difficulty, rng));
            }
            if (params.tertiarySkill > 0.0) {
                bonus += std::max(0.0, rollGaussian(params.tertiarySkill, params.difficulty, rng) / 10.0);
            }
            double power = skillCheck(params.skill, params.difficulty, params.secondaryQl, bonus, rng);
            double imbueEnhancement = 1.0 + 0.23047 * static_cast<double>(params.imbue) / 100.0;
            double itq = power * imbueEnhancement;
            if (params.materialQl < itq) {
                itq = std::max(1.0, params.materialQl);
            }
            if (params.rarity > 0.0 && power > 0.0) {
                itq = calcRareQuality(itq, params.rarity);
            }
            rollVal = itq;
            isSuccess = (power > 0.0);
            break;
        }
        case ActionMode::WoodcuttingQl: {
            double hatchetBonus = std::max(0.0, skillCheck(params.secondarySkill, params.difficulty, params.secondaryQl, 0.0, rng));
            double ql = skillCheck(params.skill, params.difficulty, params.secondaryQl, hatchetBonus, rng);
            ql = std::max(1.0, ql);
            double imbueEnhancement = 1.0 + 0.23047 * static_cast<double>(params.imbue) / 100.0;
            double woodc = params.skill * imbueEnhancement;
            if (woodc < ql) ql = woodc;
            if (std::abs(ql - 1.0) < 0.0001) {
                ql = std::max(params.skill, 1.0 + u01(rng) * 10.0 * imbueEnhancement);
            }
            rollVal = ql;
            isSuccess = (ql > 1.0);
            break;
        }
        case ActionMode::Imping: {
            double bonus = 0.0;
            if (params.secondarySkill > 0.0) {
                bonus += std::max(0.0, skillCheck(params.secondarySkill, params.targetQl, params.secondaryQl, 0.0, rng));
            }
            if (params.tertiarySkill > 0.0) {
                bonus += std::max(0.0, skillCheck(params.tertiarySkill, params.targetQl, 0.0, 0.0, rng) / 10.0);
            }
            rollVal = skillCheck(params.skill, params.targetQl, params.secondaryQl, bonus, rng);
            isSuccess = (rollVal > 0.0);
            break;
        }
        case ActionMode::Taming: {
            double targetCR = mobCr * params.tamingModifier * params.tamingAge;
            if (params.isTamed) targetCR /= 2.0;
            if (params.isHots) targetCR = 20.0;
            double bonus = std::max(0.0, skillCheck(params.secondarySkill, mobSstr + 5.0, 0.0, -targetCR, rng));
            if (params.isFo) bonus += 20.0;
            rollVal = skillCheck(params.skill, mobSstr + targetCR, 0.0, bonus, rng);
            isSuccess = (rollVal > 0.0);
            break;
        }
        case ActionMode::Fileting: {
            double knivesBonus = std::max(0.0, skillCheck(params.tertiarySkill, 10.0, 0.0, 0.0, rng));
            double bonus = skillCheck(params.secondarySkill, 10.0, 0.0, knivesBonus / 10.0, rng);
            bonus += std::max(0.0, skillCheck(params.tertiarySkill, params.difficulty, 0.0, 0.0, rng) / 10.0);
            rollVal = skillCheck(params.skill, params.difficulty, params.secondaryQl, bonus, rng);
            isSuccess = (rollVal > 0.0);
            break;
        }
        case ActionMode::Forestry: {
            double bonus = std::max(1.0, skillCheck(params.secondarySkill, 1.0, params.secondaryQl, 0.0, rng));
            rollVal = skillCheck(params.skill, params.skill - 10.0, params.secondaryQl, bonus, rng);
            isSuccess = (rollVal > 0.0);
            break;
        }
        case ActionMode::Shearing: {
            double ageDiffBonus = 5.0;
            if (params.sheepAge < 3) ageDiffBonus = 30.0;
            else if (params.sheepAge < 8) ageDiffBonus = 25.0;
            else if (params.sheepAge < 12) ageDiffBonus = 20.0;
            else if (params.sheepAge < 30) ageDiffBonus = 15.0;
            else if (params.sheepAge < 40) ageDiffBonus = 10.0;
            double diff = params.skill - ageDiffBonus;
            rollVal = skillCheck(params.skill, diff, params.secondaryQl, 0.0, rng);
            isSuccess = (rollVal > 0.0);
            break;
        }
        default:
            break;
        }

        if (isSuccess) successes++;
        sum += rollVal;
        if (rollVal < dist.min) dist.min = rollVal;
        if (rollVal > dist.max) dist.max = rollVal;
        int bucket = static_cast<int>(std::round(rollVal));
        dist.histogram[bucket]++;
    }

    dist.successRate = (static_cast<double>(successes) / trials) * 100.0;
    dist.mean = sum / trials;
    return dist;
}

} // namespace tools
