#pragma once

#include "GrangerModels.hpp"
#include <string>
#include <vector>

namespace granger {

enum class BreedingStatus {
    Compatible,
    SameAnimal,
    ParentChild,
    Siblings
};

struct BreedingResult {
    BreedingStatus status = BreedingStatus::Compatible;
    std::string message;
    std::vector<std::string> shared_traits;
    std::vector<std::string> parent1_only_traits;
    std::vector<std::string> parent2_only_traits;
    std::vector<std::string> all_traits_in_pool;
    std::vector<std::string> rare_traits_in_pool;

    bool isCompatible() const {
        return status == BreedingStatus::Compatible;
    }
};

class BreedingEvaluator {
public:
    static BreedingResult evaluate(const Animal& animal1, const Animal& animal2);
};

} // namespace granger
