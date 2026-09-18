#include <iostream>
#include <cassert>
#include <filesystem>
#include "../src/models/AnnotationStore.hpp"
#include "../src/models/ArtifactStore.hpp"
#include "../src/models/DrawingStore.hpp"

void testAnnotationStoreLoad() {
    treasure::models::AnnotationStore store("configs/annotations.json");
    auto annots = store.getByServer("Xanadu");
    assert(annots.empty() == false);
    
    std::cout << "testAnnotationStoreLoad PASSED\n";
}

void testArtifactStoreLoad() {
    treasure::models::ArtifactStore store("configs/artifacts.json");
    treasure::models::Point pt;
    bool found = store.getCaster("Chaos", pt);
    assert(found == true);
    std::cout << "testArtifactStoreLoad PASSED\n";
}

void testDrawingStoreLoad() {
    treasure::models::DrawingStore store("configs/drawings.json");
    auto objs = store.getByServer("Xanadu");
    // Depending on what's in the json this might be empty or not, but it shouldn't crash.
    std::cout << "testDrawingStoreLoad PASSED\n";
}

int main() {
    std::cout << "Running Store Tests...\n";
    testAnnotationStoreLoad();
    testArtifactStoreLoad();
    testDrawingStoreLoad();
    std::cout << "All tests passed!\n";
    return 0;
}
