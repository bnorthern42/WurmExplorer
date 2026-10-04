#include "GrangerTraits.hpp"
#include <algorithm>
#include <cctype>

namespace granger {

static const std::vector<TraitInfo> PRIMARY_TRAITS = {
    // Combat (6)
    { "BRAVE", "Brave", "It will fight fiercely.", TraitCategory::COMBAT },
    { "TOUGH", "Tough", "It is a tough bugger.", TraitCategory::COMBAT },
    { "FRIENDLY", "Friendly", "It looks more friendly than normal.", TraitCategory::COMBAT },
    { "LONGER_LOYAL", "Longer Loyal", "It is especially loyal.", TraitCategory::COMBAT },
    { "EASYTAME", "Easy Tame", "It seems more friendly.", TraitCategory::COMBAT },
    { "TAME", "Tame (Rare)", "It seems extremely tame.", TraitCategory::COMBAT },

    // Speed (5)
    { "FAST", "Fast", "It has fleeter movement than normal.", TraitCategory::SPEED },
    { "LIGHTNING", "Lightning", "It has lightning movement.", TraitCategory::SPEED },
    { "JUMPER", "Jumper", "It has very strong leg muscles.", TraitCategory::SPEED },
    { "WATER_DRAWN", "Water Drawn", "It seems accustomed to water.", TraitCategory::SPEED },
    { "RARE_SPEED", "Rare Speed (Rare)", "It is unbelievably fast.", TraitCategory::SPEED },

    // Draft (6)
    { "STRONG", "Strong", "It has a strong body.", TraitCategory::DRAFT },
    { "PACK", "Pack", "It can carry more than average.", TraitCategory::DRAFT },
    { "EXTRA_CARRY", "Extra Carry", "It has strong legs.", TraitCategory::DRAFT },
    { "EASY_GEAR", "Easy Gear", "It is easy on its gear.", TraitCategory::DRAFT },
    { "MAXSLOPE", "Max Slope (Rare)", "It seems more nimble than normal.", TraitCategory::DRAFT },
    { "STRONGER", "Stronger (Rare)", "It seems stronger than normal.", TraitCategory::DRAFT },

    // Output (6)
    { "RESOURCES_POSITIVE", "Resource Positive", "It gives more resources.", TraitCategory::OUTPUT },
    { "PRIZE_WINNING", "Prize Winning", "It seems prize winning.", TraitCategory::OUTPUT },
    { "PLUMP", "Plump", "It looks plump and ready to butcher.", TraitCategory::OUTPUT },
    { "PICKUP", "Pickup", "It seems to pick stuff up.", TraitCategory::OUTPUT },
    { "OUTPUT", "Output", "It seems vibrant.", TraitCategory::OUTPUT },
    { "GOOD_GENES", "Good Genes (Rare)", "It has very good genes.", TraitCategory::OUTPUT },

    // Misc (7)
    { "DISEASE_AVERTED", "Disease Averted", "It looks unusually strong and healthy.", TraitCategory::MISC },
    { "LONG_LIVED", "Long Lived", "It has a certain spark in its eyes.", TraitCategory::MISC },
    { "LIGHT_FOOT", "Light Foot", "It seems to be a graceful eater.", TraitCategory::MISC },
    { "STATIONERY", "Stationery", "It looks stationary.", TraitCategory::MISC },
    { "EATS_RARE", "Eats Rare", "It has a slow metabolism.", TraitCategory::MISC },
    { "IMMORTAL", "Immortal (Rare)", "It seems immortal.", TraitCategory::MISC },
    { "FERTILE", "Fertile (Rare)", "It has a chance to produce twins.", TraitCategory::MISC },

    // Negative (4 Core)
    { "SLOW", "Slow", "It has malformed hindlegs.", TraitCategory::NEGATIVE },
    { "HALT", "Halt", "The legs are of different length.", TraitCategory::NEGATIVE },
    { "BITER", "Biter", "It seems overly aggressive.", TraitCategory::NEGATIVE },
    { "NAG", "Nag", "It looks very unmotivated.", TraitCategory::NEGATIVE }
};

static const std::vector<TraitInfo> EXTENDED_TRAITS = {
    { "SURLY", "Surly", "It is unusually strong-willed.", TraitCategory::NEGATIVE },
    { "SICK", "Sick", "It has some illness.", TraitCategory::NEGATIVE },
    { "RAVAGING", "Ravaging", "It looks constantly hungry.", TraitCategory::NEGATIVE },
    { "DISEASE_PRONE", "Disease Prone", "It looks feeble and unhealthy.", TraitCategory::NEGATIVE },
    { "EXTRA_SICK", "Extra Sick", "It looks extremely sick.", TraitCategory::NEGATIVE },
    { "RESOURCES_NEGATIVE", "Resource Negative", "It looks shabby and frail.", TraitCategory::NEGATIVE },
    { "LESS_MAXSLOPE", "Less Max Slope", "It seems to dislike steep terrain.", TraitCategory::NEGATIVE }
};

const std::vector<TraitInfo>& GrangerTraits::getAllTraits() {
    return PRIMARY_TRAITS;
}

const TraitInfo* GrangerTraits::findTrait(const std::string& internalName) {
    for (const auto& t : PRIMARY_TRAITS) {
        if (internalName == t.internal_name) return &t;
    }
    for (const auto& t : EXTENDED_TRAITS) {
        if (internalName == t.internal_name) return &t;
    }
    return nullptr;
}

bool GrangerTraits::isRare(const std::string& internalName) {
    const auto* info = findTrait(internalName);
    if (!info) return false;
    std::string disp(info->display_name);
    return disp.find("(Rare)") != std::string::npos;
}

std::string GrangerTraits::formatDisplayName(const std::string& internalName) {
    const auto* info = findTrait(internalName);
    if (info) return info->display_name;

    std::string s = internalName;
    std::replace(s.begin(), s.end(), '_', ' ');
    bool capitalizeNext = true;
    for (char& c : s) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            capitalizeNext = true;
        } else if (capitalizeNext) {
            c = std::toupper(static_cast<unsigned char>(c));
            capitalizeNext = false;
        } else {
            c = std::tolower(static_cast<unsigned char>(c));
        }
    }
    return s;
}

std::string GrangerTraits::getCategoryName(TraitCategory category) {
    switch (category) {
        case TraitCategory::COMBAT: return "Combat";
        case TraitCategory::SPEED: return "Speed";
        case TraitCategory::DRAFT: return "Draft / Carry";
        case TraitCategory::OUTPUT: return "Output / Yield";
        case TraitCategory::MISC: return "Miscellaneous";
        case TraitCategory::NEGATIVE: return "Negative";
    }
    return "Unknown";
}

std::vector<TraitInfo> GrangerTraits::getTraitsByCategory(TraitCategory category) {
    std::vector<TraitInfo> result;
    for (const auto& t : PRIMARY_TRAITS) {
        if (t.category == category) result.push_back(t);
    }
    for (const auto& t : EXTENDED_TRAITS) {
        if (t.category == category) result.push_back(t);
    }
    return result;
}

std::string GrangerTraits::getFormattedTraitsHtml(const std::vector<std::string>& traitNames) {
    if (traitNames.empty()) return "<span style='color: #a1a1aa; font-style: italic;'>None</span>";

    std::string html;
    bool first = true;
    for (const auto& name : traitNames) {
        if (!first) html += " &bull; ";
        first = false;

        const auto* trait = findTrait(name);
        bool rare = isRare(name);
        bool negative = trait && trait->category == TraitCategory::NEGATIVE;

        if (rare) {
            html += "<span style='color: #37efba; font-weight: bold;'>[RARE] ";
            html += (trait ? trait->display_name : name.c_str());
            html += "</span>";
        } else if (negative) {
            html += "<span style='color: #ef4444;'>";
            html += (trait ? trait->display_name : name.c_str());
            html += "</span>";
        } else {
            html += "<span style='color: #10b981;'>";
            html += (trait ? trait->display_name : name.c_str());
            html += "</span>";
        }
    }
    return html;
}

} // namespace granger
