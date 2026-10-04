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

    // Corner grid dimensions: plateau corners + 2 * r slope radius
    const int plateauCols = topW + 1;
    const int plateauRows = topL + 1;
    res.plateauCornersX = plateauCols;
    res.plateauCornersY = plateauRows;

    const int baseCornersX = plateauCols + (2 * r);
    const int baseCornersY = plateauRows + (2 * r);

    res.cornerW = baseCornersX;
    res.cornerL = baseCornersY;
    res.baseW = baseCornersX - 1; // topW + 2 * r
    res.baseL = baseCornersY - 1; // topL + 2 * r

    const int startX = r;
    const int startY = r;
    const int endX = startX + plateauCols - 1;
    const int endY = startY + plateauRows - 1;

    res.cornerGrid.assign(res.cornerL, std::vector<int>(res.cornerW, 0));

    long long dirt = 0;
    for (int y = 0; y < res.cornerL; ++y) {
        for (int x = 0; x < res.cornerW; ++x) {
            int dx = distToInterval(x, startX, endX);
            int dy = distToInterval(y, startY, endY);
            int d = dx + dy; // Manhattan distance from plateau corners

            int h = targetHeight - maxSlope * d;
            int hi = (h > 0) ? h : 0;
            res.cornerGrid[y][x] = hi;
            dirt += hi;
        }
    }

    // Explicitly guarantee central plateau block is exactly plateauCols x plateauRows at targetHeight
    for (int pr = 0; pr < plateauRows; ++pr) {
        for (int pc = 0; pc < plateauCols; ++pc) {
            res.cornerGrid[startY + pr][startX + pc] = targetHeight;
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

int BridgePillarCalculator::getElevationTier(int height) {
    if (height <= 0) return 0;
    return (height - 1) / 300 + 1;
}

ElevationStyle BridgePillarCalculator::getElevationStyle(int height) {
    int tier = getElevationTier(height);

    ElevationStyle style;
    style.tier = tier;

    switch (tier) {
        case 0:
            // Ground base level (Dark slate)
            style.fill = { 32, 33, 36 };      // #202124 (surface dark)
            style.border = { 56, 58, 66 };    // #383a42 (subtle slate border)
            style.text = { 113, 113, 122 };   // #71717a (muted secondary text)
            break;
        case 1:
            // 1 - 300 dirt: Deep Blue
            style.fill = { 24, 49, 83 };       // #183153 deep blue
            style.border = { 59, 130, 246 };   // #3b82f6 vibrant blue
            style.text = { 244, 244, 245 };
            break;
        case 2:
            // 301 - 600 dirt: Cyan / Teal
            style.fill = { 14, 68, 85 };       // #0e4455 deep cyan
            style.border = { 6, 182, 212 };    // #06b6d4 vibrant cyan
            style.text = { 244, 244, 245 };
            break;
        case 3:
            // 601 - 900 dirt: Green / Emerald
            style.fill = { 13, 65, 47 };       // #0d412f deep emerald
            style.border = { 16, 185, 129 };   // #10b981 vibrant emerald green
            style.text = { 244, 244, 245 };
            break;
        case 4:
            // 901 - 1200 dirt: Amber / Yellow
            style.fill = { 85, 60, 15 };       // #553c0f deep amber
            style.border = { 245, 158, 11 };   // #f59e0b vibrant amber
            style.text = { 244, 244, 245 };
            break;
        case 5:
            // 1201 - 1500 dirt: Orange
            style.fill = { 88, 38, 15 };       // #58260f deep orange
            style.border = { 249, 115, 22 };   // #f97316 vibrant orange
            style.text = { 244, 244, 245 };
            break;
        case 6:
            // 1501 - 1800 dirt: Red / Crimson
            style.fill = { 92, 24, 24 };       // #5c1818 deep crimson
            style.border = { 239, 68, 68 };    // #ef4444 vibrant red
            style.text = { 244, 244, 245 };
            break;
        case 7:
            // 1801 - 2100 dirt: Rose
            style.fill = { 88, 21, 48 };       // #581530 deep rose
            style.border = { 244, 63, 94 };    // #f43f5e vibrant rose
            style.text = { 244, 244, 245 };
            break;
        case 8:
            // 2101 - 2400 dirt: Fuchsia
            style.fill = { 66, 18, 74 };       // #42124a deep fuchsia
            style.border = { 217, 70, 239 };   // #d946ef vibrant fuchsia
            style.text = { 244, 244, 245 };
            break;
        case 9:
            // 2401 - 2700 dirt: Purple
            style.fill = { 52, 20, 82 };       // #341452 deep purple
            style.border = { 168, 85, 247 };   // #a855f7 vibrant purple
            style.text = { 244, 244, 245 };
            break;
        default:
            // 2701+ dirt: Mint highlight
            style.fill = { 13, 56, 41 };       // #0d3829 deep mint
            style.border = { 55, 239, 186 };   // #37efba vibrant mint
            style.text = { 18, 18, 20 };
            break;
    }

    return style;
}

} // namespace tools
