#pragma once

#include <vector>
#include <optional>

namespace tools {

struct PillarResult {
    int topW = 0;
    int topL = 0;
    int targetHeight = 0;
    double effectiveSlope = 300.0;
    int spreadRadius = 0;
    int plateauCornersX = 0;
    int plateauCornersY = 0;
    int baseW = 0;
    int baseL = 0;
    int cornerW = 0;
    int cornerL = 0;
    long long totalDirt = 0;
    long long crates = 0;
    std::vector<std::vector<int>> cornerGrid; // [CL][CW]
    std::vector<std::vector<int>> tileGrid;   // [baseL][baseW]
};

class BridgePillarCalculator {
public:
    static PillarResult calculate(int topW, int topL, int targetHeight, std::optional<double> digSkill = std::nullopt);
};

} // namespace tools
