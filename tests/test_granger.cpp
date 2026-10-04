#include <iostream>
#include <cassert>
#include <filesystem>
#include <algorithm>
#include "../src/features/granger/GrangerTraits.hpp"
#include "../src/features/granger/GrangerModels.hpp"
#include "../src/features/granger/BreedingEvaluator.hpp"
#include "../src/features/granger/GrangerStore.hpp"

using namespace granger;

void testTraitsCatalog() {
    const auto& all = GrangerTraits::getAllTraits();
    assert(all.size() == 34);

    const TraitInfo* fast = GrangerTraits::findTrait("FAST");
    assert(fast != nullptr);
    assert(fast->category == TraitCategory::SPEED);
    assert(std::string(fast->display_name) == "Fast");
    assert(!GrangerTraits::isRare("FAST"));

    const TraitInfo* rareSpeed = GrangerTraits::findTrait("RARE_SPEED");
    assert(rareSpeed != nullptr);
    assert(GrangerTraits::isRare("RARE_SPEED"));

    const TraitInfo* brave = GrangerTraits::findTrait("BRAVE");
    assert(brave != nullptr);
    assert(brave->category == TraitCategory::COMBAT);

    const TraitInfo* slow = GrangerTraits::findTrait("SLOW");
    assert(slow != nullptr);
    assert(slow->category == TraitCategory::NEGATIVE);

    assert(GrangerTraits::formatDisplayName("LONGER_LOYAL") == "Longer Loyal");
    assert(GrangerTraits::formatDisplayName("WATER_DRAWN") == "Water Drawn");

    std::cout << "testTraitsCatalog PASSED\n";
}

void testPlayerAndSlots() {
    Player p1;
    p1.id = 1;
    p1.name = "polarbear";
    p1.ah_skill = 51;
    p1.extra_slots = 0;
    assert(p1.maxSlots() == 6); // 1 + floor(51/10) + 0 = 6

    Player p2;
    p2.id = 2;
    p2.name = "breeder2";
    p2.ah_skill = 0;
    p2.extra_slots = 0;
    assert(p2.maxSlots() == 1); // 1 + 0 + 0 = 1

    Player p3;
    p3.id = 3;
    p3.name = "champion";
    p3.ah_skill = 95;
    p3.extra_slots = 3;
    assert(p3.maxSlots() == 13); // 1 + 9 + 3 = 13

    std::cout << "testPlayerAndSlots PASSED\n";
}

void testBreedingEvaluator() {
    Animal a1; a1.id = 1; a1.name = "Sire"; a1.mother_id = 0; a1.father_id = 0;
    a1.traits = {"FAST", "LIGHTNING", "STRONG"};

    Animal a2; a2.id = 2; a2.name = "Dam"; a2.mother_id = 0; a2.father_id = 0;
    a2.traits = {"FAST", "PACK", "RARE_SPEED"};

    // 1. Same animal
    auto resSame = BreedingEvaluator::evaluate(a1, a1);
    assert(resSame.status == BreedingStatus::SameAnimal);
    assert(!resSame.isCompatible());

    // 2. Parent-child (Dam is daughter of Sire)
    Animal daughter; daughter.id = 3; daughter.name = "Daughter";
    daughter.mother_id = 2; daughter.father_id = 1;
    auto resPC1 = BreedingEvaluator::evaluate(a1, daughter);
    assert(resPC1.status == BreedingStatus::ParentChild);
    assert(!resPC1.isCompatible());

    auto resPC2 = BreedingEvaluator::evaluate(daughter, a2);
    assert(resPC2.status == BreedingStatus::ParentChild);
    assert(!resPC2.isCompatible());

    // 3. Siblings (Same mother)
    Animal son; son.id = 4; son.name = "Son";
    son.mother_id = 2; son.father_id = 99;
    auto resSib1 = BreedingEvaluator::evaluate(daughter, son);
    assert(resSib1.status == BreedingStatus::Siblings);
    assert(!resSib1.isCompatible());

    // 4. Siblings (Same father)
    Animal halfBro; halfBro.id = 5; halfBro.name = "HalfBro";
    halfBro.mother_id = 88; halfBro.father_id = 1;
    auto resSib2 = BreedingEvaluator::evaluate(daughter, halfBro);
    assert(resSib2.status == BreedingStatus::Siblings);
    assert(!resSib2.isCompatible());

    // 5. Compatible unrelated pair
    auto resComp = BreedingEvaluator::evaluate(a1, a2);
    assert(resComp.status == BreedingStatus::Compatible);
    assert(resComp.isCompatible());

    // Verify trait breakdown
    assert(resComp.shared_traits.size() == 1);
    assert(resComp.shared_traits[0] == "FAST");

    assert(resComp.parent1_only_traits.size() == 2);
    assert(std::find(resComp.parent1_only_traits.begin(), resComp.parent1_only_traits.end(), "LIGHTNING") != resComp.parent1_only_traits.end());
    assert(std::find(resComp.parent1_only_traits.begin(), resComp.parent1_only_traits.end(), "STRONG") != resComp.parent1_only_traits.end());

    assert(resComp.parent2_only_traits.size() == 2);
    assert(std::find(resComp.parent2_only_traits.begin(), resComp.parent2_only_traits.end(), "PACK") != resComp.parent2_only_traits.end());
    assert(std::find(resComp.parent2_only_traits.begin(), resComp.parent2_only_traits.end(), "RARE_SPEED") != resComp.parent2_only_traits.end());

    assert(resComp.rare_traits_in_pool.size() == 1);
    assert(resComp.rare_traits_in_pool[0] == "RARE_SPEED");

    std::cout << "testBreedingEvaluator PASSED\n";
}

void testGrangerStorePersistence() {
    std::string testPath = "/tmp/test_granger_store.json";
    if (std::filesystem::exists(testPath)) {
        std::filesystem::remove(testPath);
    }

    {
        GrangerStore store(testPath);
        Player p;
        p.id = 1;
        p.name = "polarbear";
        p.ah_skill = 51;
        p.extra_slots = 0;
        store.addPlayer(p);

        Animal a1;
        a1.id = 10;
        a1.name = "Heartflea";
        a1.type = "Champion Horse";
        a1.traits = {"FAST", "LIGHTNING", "JUMPER", "WATER_DRAWN"};
        a1.cared_by = 1;
        store.addAnimal(a1);

        Animal a2;
        a2.id = 11;
        a2.name = "Bloodland";
        a2.type = "horse";
        a2.traits = {"STRONG", "PACK", "EXTRA_CARRY", "EASY_GEAR"};
        a2.cared_by = 0;
        store.addAnimal(a2);

        assert(store.getPlayers().size() == 1);
        assert(store.getAnimals().size() == 2);
        assert(store.getCaredAnimalCount(1) == 1);
        assert(store.getRemainingSlots(1) == 5); // 6 - 1 = 5

        store.save();
    }

    // Reload from file and verify
    {
        GrangerStore store(testPath);
        assert(store.getPlayers().size() == 1);
        assert(store.getAnimals().size() == 2);

        const Player* p = store.getPlayer(1);
        assert(p != nullptr);
        assert(p->name == "polarbear");
        assert(p->ah_skill == 51);

        const Animal* a1 = store.getAnimal(10);
        assert(a1 != nullptr);
        assert(a1->name == "Heartflea");
        assert(a1->traits.size() == 4);
        assert(a1->cared_by == 1);

        // Test update animal
        Animal updated = *a1;
        updated.name = "Heartflea Prime";
        updated.traits.push_back("BRAVE");
        store.updateAnimal(updated);

        const Animal* checkUpdated = store.getAnimal(10);
        assert(checkUpdated->name == "Heartflea Prime");
        assert(checkUpdated->traits.size() == 5);

        // Test delete animal
        bool deleted = store.deleteAnimal(11);
        assert(deleted);
        assert(store.getAnimals().size() == 1);
        assert(store.getAnimal(11) == nullptr);

        store.save();
    }

    // Verify delete persisted
    {
        GrangerStore store(testPath);
        assert(store.getAnimals().size() == 1);
        assert(store.getAnimal(10)->name == "Heartflea Prime");
        assert(store.getAnimal(11) == nullptr);
    }

    std::filesystem::remove(testPath);
    std::cout << "testGrangerStorePersistence PASSED\n";
}

int main() {
    std::cout << "Running Granger Tests...\n";
    testTraitsCatalog();
    testPlayerAndSlots();
    testBreedingEvaluator();
    testGrangerStorePersistence();
    std::cout << "All Granger tests passed!\n";
    return 0;
}
