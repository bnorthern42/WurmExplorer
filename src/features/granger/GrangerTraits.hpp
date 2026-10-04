#pragma once

#include <string>
#include <vector>
#include <map>

namespace granger {

enum class TraitCategory {
    COMBAT,
    SPEED,
    DRAFT,
    OUTPUT,
    MISC,
    NEGATIVE
};

struct TraitInfo {
    const char* internal_name;
    const char* display_name;
    const char* description;
    TraitCategory category;
};

class GrangerTraits {
public:
    static const std::vector<TraitInfo>& getAllTraits();
    static const TraitInfo* findTrait(const std::string& internalName);
    static bool isRare(const std::string& internalName);
    static std::string formatDisplayName(const std::string& internalName);
    static std::string getCategoryName(TraitCategory category);
    static std::vector<TraitInfo> getTraitsByCategory(TraitCategory category);
    static std::string getFormattedTraitsHtml(const std::vector<std::string>& traitNames);
};

} // namespace granger
