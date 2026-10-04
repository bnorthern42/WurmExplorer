#pragma once

namespace tools {

class ImpCalculator {
public:
    // Calculates maximum impable QL given current skill (and optional imbue bonus)
    // Formula: (skill * 0.77) + 23
    static double calculateMaxImpQl(double skill, int imbue = 0);

    // Calculates skill required to achieve a desired target QL
    // Formula: (targetQl - 23) / 0.77
    static double calculateSkillNeeded(double targetQl, int imbue = 0);
};

} // namespace tools
