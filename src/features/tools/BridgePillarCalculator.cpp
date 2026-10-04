#include "BridgePillarCalculator.hpp"
#include <algorithm>
#include <cmath>

namespace tools {

static int distToInterval(int v, int lo, int hi) {
    if (v < lo) return lo - v;
    if (v > hi) return v - hi;
    return 0;
}

PillarResult BridgePillarCalculator::calculate(int topW, int topL, int targetHeight, std::optional<double> digSkill) {
    PillarResult res;
    if (topW <= 0 || topL <= 0 || targetHeight < 0) {
        return res;
    }

    res.topW = topW;
    res.topL = topL;
    res.targetHeight = targetHeight;

    int maxSlope = 300;
    if (digSkill.has_value()) {
        maxSlope = static_cast<int>(std::floor(*digSkill * 3.0));
        maxSlope = std::clamp(maxSlope, 1, 300);
    }
    res.effectiveSlope = static_cast<double>(maxSlope);

    const int r = (targetHeight + maxSlope - 1) / maxSlope;
    res.spreadRadius = r;

    res.baseW = topW + 2 * r;
    res.baseL = topL + 2 * r;

    res.cornerW = res.baseW + 1;
    res.cornerL = res.baseL + 1;

    const int px0 = r;
    const int py0 = r;
    const int px1 = r + topW;
    const int py1 = r + topL;

    res.cornerGrid.assign(res.cornerL, std::vector<int>(res.cornerW, 0));

    long long dirt = 0;
    for (int y = 0; y < res.cornerL; ++y) {
        for (int x = 0; x < res.cornerW; ++x) {
            int dx = distToInterval(x, px0, px1);
            int dy = distToInterval(y, py0, py1);
            int d = dx + dy; // Manhattan distance from plateau corners

            int h = targetHeight - maxSlope * d;
            int hi = (h > 0) ? h : 0;
            res.cornerGrid[y][x] = hi;
            dirt += hi;
        }
    }

    res.totalDirt = dirt;
    res.crates = (dirt + 300 - 1) / 300;

    res.tileGrid.assign(res.baseL, std::vector<int>(res.baseW, 0));
    for (int y = 0; y < res.baseL; ++y) {
        for (int x = 0; x < res.baseW; ++x) {
            int t = std::max({
                res.cornerGrid[y][x],
                res.cornerGrid[y][x + 1],
                res.cornerGrid[y + 1][x],
                res.cornerGrid[y + 1][x + 1]
            });
            res.tileGrid[y][x] = t;
        }
    }

    return res;
}

} // namespace tools
