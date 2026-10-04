#include "DifficultyPresets.hpp"
#include <unordered_map>

namespace tools {

static const std::unordered_map<ActionMode, std::vector<DifficultyPreset>> s_presets = {
    {
        ActionMode::GenericCheck,
        {
            {"Very Easy", 5.0},
            {"Easy Task", 10.0},
            {"Standard Task", 20.0},
            {"Challenging Task", 40.0},
            {"Hard Task", 60.0},
            {"Extreme Task", 80.0}
        }
    },
    {
        ActionMode::MiningPower,
        {
            {"Copper Vein", 10.0},
            {"Iron Vein", 20.0},
            {"Tin Vein", 20.0},
            {"Lead / Zinc", 25.0},
            {"Silver Vein", 30.0},
            {"Gold Vein", 40.0}
        }
    },
    {
        ActionMode::MiningQl,
        {
            {"Rock Tile", 10.0},
            {"Iron Vein", 20.0},
            {"Tin Vein", 20.0},
            {"Slate Vein", 15.0},
            {"Silver Vein", 30.0},
            {"Marble Vein", 35.0},
            {"Gold Vein", 40.0}
        }
    },
    {
        ActionMode::Farming,
        {
            {"Corn / Rye / Wheat", 10.0},
            {"Barley / Oat", 15.0},
            {"Potato / Carrot", 20.0},
            {"Cotton / Hemp", 25.0},
            {"Strawberries", 30.0},
            {"Garlic", 40.0}
        }
    },
    {
        ActionMode::Digging,
        {
            {"Dirt / Grass", 10.0},
            {"Clay / Peat", 20.0},
            {"Tar / Moss", 30.0},
            {"Underwater Dirt", 40.0},
            {"Steep Slope", 50.0}
        }
    },
    {
        ActionMode::Meditation,
        {
            {"Level 1 Path", 10.0},
            {"Level 3 Path", 20.0},
            {"Level 5 Path", 30.0},
            {"Level 7 Path", 45.0},
            {"Level 9 Path", 60.0},
            {"Level 11 Path", 75.0}
        }
    },
    {
        ActionMode::Creation,
        {
            {"Shaft / Handle", 5.0},
            {"Simple Tool", 10.0},
            {"Carving Knife", 20.0},
            {"Large Cart", 30.0},
            {"Small Wooden Shacks", 40.0},
            {"Rowing Boat", 50.0},
            {"Corbita", 70.0}
        }
    },
    {
        ActionMode::WoodcuttingQl,
        {
            {"Pine / Birch", 15.0},
            {"Cedar / Fir", 20.0},
            {"Oak / Willow", 30.0},
            {"Walnut / Chestnut", 35.0},
            {"Linden / Maple", 40.0}
        }
    },
    {
        ActionMode::Imping,
        {
            {"Low QL < 30", 10.0},
            {"Mid QL 30-60", 25.0},
            {"High QL 60-80", 45.0},
            {"Master QL 80-90", 65.0},
            {"Supreme QL 90+", 85.0}
        }
    },
    {
        ActionMode::SmithingSteps,
        {
            {"Pelt / Needle", 5.0},
            {"Carving Knife Blade", 15.0},
            {"Horseshoe", 20.0},
            {"Large Anvil", 30.0},
            {"Longsword", 40.0},
            {"Plate Armor", 60.0}
        }
    },
    {
        ActionMode::Taming,
        {
            {"Chicken", 5.0},
            {"Pig", 15.0},
            {"Cow", 20.0},
            {"Dog", 25.0},
            {"Bull", 30.0},
            {"Brown Bear", 40.0},
            {"Crocodile", 50.0},
            {"Black Bear", 55.0},
            {"Cave Bug", 65.0},
            {"Hell Horse", 80.0},
            {"Unicorn", 90.0}
        }
    },
    {
        ActionMode::Fileting,
        {
            {"Roach / Perch", 10.0},
            {"Trout / Bass", 20.0},
            {"Pike / Catfish", 30.0},
            {"Shark / Marlin", 50.0}
        }
    },
    {
        ActionMode::Forestry,
        {
            {"Sprout Picking", 10.0},
            {"Pruning Young Tree", 20.0},
            {"Pruning Mature Tree", 30.0},
            {"Pruning Old Tree", 45.0}
        }
    },
    {
        ActionMode::Shearing,
        {
            {"Lamb", 10.0},
            {"Young Sheep", 15.0},
            {"Adult Sheep", 20.0},
            {"Overgrown Fleece", 30.0}
        }
    }
};

std::vector<DifficultyPreset> DifficultyProvider::getPresetsForMode(ActionMode mode) {
    auto it = s_presets.find(mode);
    if (it != s_presets.end()) {
        return it->second;
    }
    return {};
}

bool DifficultyProvider::hasPresetsForMode(ActionMode mode) {
    auto it = s_presets.find(mode);
    return it != s_presets.end() && !it->second.empty();
}

} // namespace tools
