#include "DifficultyPresets.hpp"
#include <unordered_map>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <mutex>

namespace tools {

static const std::unordered_map<std::string, ActionMode> s_modeMap = {
    {"GenericCheck", ActionMode::GenericCheck},
    {"MiningPower", ActionMode::MiningPower},
    {"MiningQl", ActionMode::MiningQl},
    {"Farming", ActionMode::Farming},
    {"Digging", ActionMode::Digging},
    {"Meditation", ActionMode::Meditation},
    {"Creation", ActionMode::Creation},
    {"WoodcuttingQl", ActionMode::WoodcuttingQl},
    {"Imping", ActionMode::Imping},
    {"SmithingSteps", ActionMode::SmithingSteps},
    {"Taming", ActionMode::Taming},
    {"Fileting", ActionMode::Fileting},
    {"Forestry", ActionMode::Forestry},
    {"Shearing", ActionMode::Shearing}
};

static const std::unordered_map<ActionMode, std::vector<DifficultyPreset>> s_fallbackPresets = {
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
            {"Stone Vein", 2.0},
            {"Zinc Vein", 2.0},
            {"Iron Vein", 3.0},
            {"Tin Vein", 10.0},
            {"Copper Vein", 20.0},
            {"Slate Vein", 20.0},
            {"Lead Vein", 20.0},
            {"Silver Vein", 35.0},
            {"Gold Vein", 40.0},
            {"Reinforced Vein", 40.0},
            {"Marble Vein", 40.0},
            {"Sandstone Vein", 45.0},
            {"Glimmersteel Vein", 55.0},
            {"Adamantine Vein", 60.0}
        }
    },
    {
        ActionMode::MiningQl,
        {
            {"Stone Vein", 2.0},
            {"Zinc Vein", 2.0},
            {"Iron Vein", 3.0},
            {"Tin Vein", 10.0},
            {"Copper Vein", 20.0},
            {"Slate Vein", 20.0},
            {"Lead Vein", 20.0},
            {"Silver Vein", 35.0},
            {"Gold Vein", 40.0},
            {"Reinforced Vein", 40.0},
            {"Marble Vein", 40.0},
            {"Sandstone Vein", 45.0},
            {"Glimmersteel Vein", 55.0},
            {"Adamantine Vein", 60.0}
        }
    },
    {
        ActionMode::Farming,
        {
            {"Potato", 4.0},
            {"Cotton", 7.0},
            {"Rye", 10.0},
            {"Barley", 20.0},
            {"Wheat", 30.0},
            {"Corn", 40.0}
        }
    },
    {
        ActionMode::Digging,
        {
            {"Moss", 10.0},
            {"Sand", 10.0},
            {"Clay", 20.0},
            {"Tundra", 20.0},
            {"Marsh", 30.0},
            {"Tar", 35.0},
            {"Steppe", 40.0}
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
            {"Birch Tree", 2.0},
            {"Pine Tree", 2.0},
            {"Maple Tree", 4.0},
            {"Cedar Tree", 5.0},
            {"Oak Tree", 20.0},
            {"Willow Tree", 18.0}
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
            {"Lamb", 5.0},
            {"Brown Cow", 10.0},
            {"Cow", 10.0},
            {"Cow (Young)", 9.0},
            {"Cow (Adolescent)", 10.0},
            {"Cow (Mature)", 11.0},
            {"Cow (Aged)", 12.0},
            {"Cow (Old)", 13.0},
            {"Cow (Venerable)", 14.0},
            {"Pig", 20.0},
            {"Pig (Young)", 18.0},
            {"Pig (Mature)", 22.0},
            {"Dog", 45.0},
            {"Dog (Young)", 40.5},
            {"Dog (Mature)", 49.5},
            {"Horse", 132.0},
            {"Horse (Young)", 118.8},
            {"Horse (Adolescent)", 132.0},
            {"Horse (Mature)", 145.2},
            {"Horse (Aged)", 158.4},
            {"Horse (Old)", 171.6},
            {"Horse (Venerable)", 184.8},
            {"Cave Bug", 200.0},
            {"Brown Bear", 270.0},
            {"Black Bear", 270.0},
            {"Crocodile", 585.0},
            {"Hell Horse", 648.0},
            {"Hell Horse (Young)", 583.2},
            {"Hell Horse (Mature)", 712.8},
            {"Unicorn", 660.0}
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
            {"Lamb (< 3 yrs)", 10.0},
            {"Young Sheep (< 8 yrs)", 15.0},
            {"Adolescent Sheep (< 12 yrs)", 20.0},
            {"Adult Sheep (< 30 yrs)", 25.0},
            {"Mature Sheep (< 40 yrs)", 30.0},
            {"Old / Overgrown Fleece", 35.0}
        }
    }
};

static std::unordered_map<ActionMode, std::vector<DifficultyPreset>> s_jsonPresets;
static std::once_flag s_once;

static void loadJsonPresets() {
    const QStringList candidates = {
        "configs/difficulty_presets.json",
        "../configs/difficulty_presets.json",
        "../../configs/difficulty_presets.json"
    };
    for (const auto& path : candidates) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                QJsonObject root = doc.object();
                for (auto it = root.begin(); it != root.end(); ++it) {
                    auto mIt = s_modeMap.find(it.key().toStdString());
                    if (mIt != s_modeMap.end()) {
                        ActionMode mode = mIt->second;
                        if (it.value().isArray()) {
                            QJsonArray arr = it.value().toArray();
                            std::vector<DifficultyPreset> list;
                            for (const auto& itemVal : arr) {
                                if (itemVal.isObject()) {
                                    QJsonObject obj = itemVal.toObject();
                                    DifficultyPreset dp;
                                    dp.name = obj.value("name").toString();
                                    dp.difficulty = obj.value("difficulty").toDouble();
                                    list.push_back(dp);
                                }
                            }
                            if (!list.empty()) {
                                s_jsonPresets[mode] = std::move(list);
                            }
                        }
                    }
                }
                return;
            }
        }
    }
}

std::vector<DifficultyPreset> DifficultyProvider::getPresetsForMode(ActionMode mode) {
    std::call_once(s_once, loadJsonPresets);
    auto it = s_jsonPresets.find(mode);
    if (it != s_jsonPresets.end() && !it->second.empty()) {
        return it->second;
    }
    auto fIt = s_fallbackPresets.find(mode);
    if (fIt != s_fallbackPresets.end()) {
        return fIt->second;
    }
    return {};
}

bool DifficultyProvider::hasPresetsForMode(ActionMode mode) {
    std::call_once(s_once, loadJsonPresets);
    auto it = s_jsonPresets.find(mode);
    if (it != s_jsonPresets.end() && !it->second.empty()) {
        return true;
    }
    auto fIt = s_fallbackPresets.find(mode);
    return fIt != s_fallbackPresets.end() && !fIt->second.empty();
}

} // namespace tools
