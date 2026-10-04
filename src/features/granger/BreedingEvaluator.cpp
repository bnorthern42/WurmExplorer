#include "BreedingEvaluator.hpp"
#include "GrangerTraits.hpp"
#include <algorithm>
#include <set>

namespace granger {

BreedingResult BreedingEvaluator::evaluate(const Animal& animal1, const Animal& animal2) {
    BreedingResult res;

    // Check relationship
    if (animal1.id == animal2.id) {
        res.status = BreedingStatus::SameAnimal;
        res.message = "Same animal";
    } else if ((animal1.mother_id && animal1.mother_id == animal2.id) ||
               (animal1.father_id && animal1.father_id == animal2.id) ||
               (animal2.mother_id && animal2.mother_id == animal1.id) ||
               (animal2.father_id && animal2.father_id == animal1.id)) {
        res.status = BreedingStatus::ParentChild;
        res.message = "Parent-child relation (Incompatible)";
    } else if ((animal1.mother_id && animal2.mother_id && animal1.mother_id == animal2.mother_id) ||
               (animal1.father_id && animal2.father_id && animal1.father_id == animal2.father_id)) {
        res.status = BreedingStatus::Siblings;
        res.message = "Siblings (Incompatible)";
    } else {
        res.status = BreedingStatus::Compatible;
        res.message = "Compatible";
    }

    // Trait pool analysis
    std::set<std::string> t1(animal1.traits.begin(), animal1.traits.end());
    std::set<std::string> t2(animal2.traits.begin(), animal2.traits.end());

    for (const auto& tr : t1) {
        if (t2.count(tr)) {
            res.shared_traits.push_back(tr);
        } else {
            res.parent1_only_traits.push_back(tr);
        }
    }

    for (const auto& tr : t2) {
        if (!t1.count(tr)) {
            res.parent2_only_traits.push_back(tr);
        }
    }

    std::set<std::string> unionTraits = t1;
    unionTraits.insert(t2.begin(), t2.end());
    for (const auto& tr : unionTraits) {
        res.all_traits_in_pool.push_back(tr);
        if (GrangerTraits::isRare(tr)) {
            res.rare_traits_in_pool.push_back(tr);
        }
    }

    return res;
}

} // namespace granger
