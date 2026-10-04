#pragma once

#include <QString>
#include <vector>
#include <string>
#include "GrinderEngine.hpp"

namespace tools {

struct ModeUiConfig {
    QString primarySkillLabel = "Primary Skill:";
    QString secondarySkillLabel = "Secondary Skill:";
    QString tertiarySkillLabel = "Tertiary Skill:";
    QString toolQlLabel = "Tool / Item QL:";
    QString difficultyLabel = "Difficulty:";

    bool showSecondarySkill = false;
    bool showTertiarySkill = false;
    bool showToolQl = false;
    bool showDifficulty = false;
    bool showMob = false;
    bool showMaterialQl = false;
    bool showStartQl = false;
    bool showTargetQl = false;
    bool showVeinQl = false;
    bool showImbue = false;
    bool showRarity = false;
    bool showRune = false;
    bool showSlope = false;
    bool showPathLevel = false;
    bool showSheepAge = false;
    bool showBenediction = false;
    bool showMediTile = false;
    bool showMediCooldown = false;
    bool showFo = false;
    bool showHots = false;
    bool showTamed = false;

    std::vector<std::string> primaryCandidates;
    std::vector<std::string> secondaryCandidates;
    std::vector<std::string> tertiaryCandidates;
};

inline ModeUiConfig getModeConfig(ActionMode mode) {
    ModeUiConfig cfg;
    switch (mode) {
    case ActionMode::GenericCheck:
        cfg.primarySkillLabel = "Skill Level:";
        cfg.toolQlLabel = "Item QL (0=none):";
        cfg.showToolQl = true;
        cfg.showDifficulty = true;
        cfg.showBenediction = true;
        cfg.primaryCandidates = {"Mining", "Carpentry", "Blacksmithing", "Digging"};
        break;
    case ActionMode::MiningPower:
        cfg.primarySkillLabel = "Mining Skill:";
        cfg.secondarySkillLabel = "Pickaxe Skill:";
        cfg.toolQlLabel = "Pickaxe QL:";
        cfg.showSecondarySkill = true;
        cfg.showToolQl = true;
        cfg.showDifficulty = true;
        cfg.primaryCandidates = {"Mining"};
        cfg.secondaryCandidates = {"Pickaxe"};
        break;
    case ActionMode::MiningQl:
        cfg.primarySkillLabel = "Mining Skill:";
        cfg.secondarySkillLabel = "Pickaxe Skill:";
        cfg.toolQlLabel = "Pickaxe QL:";
        cfg.showSecondarySkill = true;
        cfg.showToolQl = true;
        cfg.showDifficulty = true;
        cfg.showVeinQl = true;
        cfg.showImbue = true;
        cfg.showRarity = true;
        cfg.showRune = true;
        cfg.primaryCandidates = {"Mining"};
        cfg.secondaryCandidates = {"Pickaxe"};
        break;
    case ActionMode::Farming:
        cfg.primarySkillLabel = "Farming Skill:";
        cfg.secondarySkillLabel = "Rake Skill:";
        cfg.tertiarySkillLabel = "Nature Skill:";
        cfg.toolQlLabel = "Rake QL:";
        cfg.showSecondarySkill = true;
        cfg.showTertiarySkill = true;
        cfg.showToolQl = true;
        cfg.showDifficulty = true;
        cfg.primaryCandidates = {"Farming"};
        cfg.secondaryCandidates = {"Rake"};
        cfg.tertiaryCandidates = {"Natural substances", "Nature"};
        break;
    case ActionMode::Digging:
        cfg.primarySkillLabel = "Digging Skill:";
        cfg.toolQlLabel = "Shovel QL:";
        cfg.difficultyLabel = "Base Difficulty:";
        cfg.showToolQl = true;
        cfg.showDifficulty = true;
        cfg.showSlope = true;
        cfg.primaryCandidates = {"Digging"};
        break;
    case ActionMode::Meditation:
        cfg.primarySkillLabel = "Meditation Skill:";
        cfg.tertiarySkillLabel = "Natural Substances:";
        cfg.toolQlLabel = "Rug QL:";
        cfg.showTertiarySkill = true;
        cfg.showToolQl = true;
        cfg.showPathLevel = true;
        cfg.showMediTile = true;
        cfg.showMediCooldown = true;
        cfg.primaryCandidates = {"Meditation"};
        cfg.tertiaryCandidates = {"Natural substances"};
        break;
    case ActionMode::Creation:
        cfg.primarySkillLabel = "Craft Skill:";
        cfg.secondarySkillLabel = "Tool Skill:";
        cfg.tertiarySkillLabel = "Parent Skill:";
        cfg.toolQlLabel = "Tool QL:";
        cfg.showSecondarySkill = true;
        cfg.showTertiarySkill = true;
        cfg.showToolQl = true;
        cfg.showDifficulty = true;
        cfg.showMaterialQl = true;
        cfg.showImbue = true;
        cfg.showRarity = true;
        cfg.primaryCandidates = {"Carpentry", "Blacksmithing"};
        cfg.secondaryCandidates = {"Hammer", "Carving knife"};
        break;
    case ActionMode::WoodcuttingQl:
        cfg.primarySkillLabel = "Woodcutting Skill:";
        cfg.secondarySkillLabel = "Hatchet Skill:";
        cfg.toolQlLabel = "Hatchet QL:";
        cfg.showSecondarySkill = true;
        cfg.showToolQl = true;
        cfg.showDifficulty = true;
        cfg.showImbue = true;
        cfg.primaryCandidates = {"Woodcutting"};
        cfg.secondaryCandidates = {"Hatchet"};
        break;
    case ActionMode::Imping:
        cfg.primarySkillLabel = "Crafting Skill:";
        cfg.secondarySkillLabel = "Tool Skill:";
        cfg.tertiarySkillLabel = "Parent Skill:";
        cfg.toolQlLabel = "Tool QL:";
        cfg.showSecondarySkill = true;
        cfg.showTertiarySkill = true;
        cfg.showToolQl = true;
        cfg.showTargetQl = true;
        cfg.primaryCandidates = {"Blacksmithing", "Weaponsmithing", "Armoursmithing"};
        cfg.secondaryCandidates = {"Hammer"};
        break;
    case ActionMode::SmithingSteps:
        cfg.primarySkillLabel = "Smithing Skill:";
        cfg.toolQlLabel = "Tools Avg QL:";
        cfg.showToolQl = true;
        cfg.showStartQl = true;
        cfg.showTargetQl = true;
        cfg.showImbue = true;
        cfg.primaryCandidates = {"Blacksmithing", "Weaponsmithing", "Armoursmithing"};
        cfg.secondaryCandidates = {"Hammer"};
        break;
    case ActionMode::Taming:
        cfg.primarySkillLabel = "Taming Skill:";
        cfg.secondarySkillLabel = "Soul Strength:";
        cfg.showSecondarySkill = true;
        cfg.showMob = true;
        cfg.showFo = true;
        cfg.showHots = true;
        cfg.showTamed = true;
        cfg.primaryCandidates = {"Taming"};
        cfg.secondaryCandidates = {"Soul strength"};
        break;
    case ActionMode::Fileting:
        cfg.primarySkillLabel = "Butchery Skill:";
        cfg.secondarySkillLabel = "Knife Skill:";
        cfg.tertiarySkillLabel = "Cooking Skill:";
        cfg.toolQlLabel = "Knife QL:";
        cfg.showSecondarySkill = true;
        cfg.showTertiarySkill = true;
        cfg.showToolQl = true;
        cfg.showDifficulty = true;
        cfg.primaryCandidates = {"Fish butchery", "Butchery"};
        cfg.secondaryCandidates = {"Carving knife", "Knife"};
        cfg.tertiaryCandidates = {"Cooking"};
        break;
    case ActionMode::Forestry:
        cfg.primarySkillLabel = "Forestry Skill:";
        cfg.secondarySkillLabel = "Sickle Skill:";
        cfg.toolQlLabel = "Sickle QL:";
        cfg.showSecondarySkill = true;
        cfg.showToolQl = true;
        cfg.primaryCandidates = {"Forestry"};
        cfg.secondaryCandidates = {"Sickle"};
        break;
    case ActionMode::Shearing:
        cfg.primarySkillLabel = "Animal Husbandry:";
        cfg.toolQlLabel = "Scissors QL:";
        cfg.showToolQl = true;
        cfg.showSheepAge = true;
        cfg.primaryCandidates = {"Animal husbandry"};
        break;
    }
    return cfg;
}

} // namespace tools
