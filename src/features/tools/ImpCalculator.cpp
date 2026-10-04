#include "ImpCalculator.hpp"
#include <algorithm>

namespace tools {

double ImpCalculator::calculateMaxImpQl(double skill, int imbue) {
    constexpr double extra = 23.0;
    constexpr double mult = 0.77;
    double maxQl = (skill * mult) + extra;
    if (imbue > 0) {
        maxQl *= (1.0 + 0.23047 * static_cast<double>(imbue) / 100.0);
    }
    return std::min(100.0, std::max(0.0, maxQl));
}

double ImpCalculator::calculateSkillNeeded(double targetQl, int imbue) {
    constexpr double extra = 23.0;
    constexpr double mult = 0.77;
    if (targetQl < 23.77) {
        return 0.0;
    }
    double needed = (targetQl - extra) / mult;
    if (imbue > 0) {
        needed /= (1.0 + 0.23047 * static_cast<double>(imbue) / 100.0);
    }
    return std::min(100.0, std::max(0.0, needed));
}

} // namespace tools
