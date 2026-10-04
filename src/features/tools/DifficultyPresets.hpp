#pragma once

#include <QString>
#include <vector>
#include "GrinderEngine.hpp"

namespace tools {

struct DifficultyPreset {
    QString name;
    double difficulty = 0.0;
};

class DifficultyProvider {
public:
    static std::vector<DifficultyPreset> getPresetsForMode(ActionMode mode);
    static bool hasPresetsForMode(ActionMode mode);
};

} // namespace tools
