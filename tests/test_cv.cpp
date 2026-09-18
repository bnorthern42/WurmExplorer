#include <iostream>
#include <cassert>
#include <opencv2/opencv.hpp>
#include "../src/engine/cv/ImageOps.hpp"
#include "../src/engine/cv/Extractor.hpp"

using namespace treasure::cv;

void testToGray() {
    cv::Mat bgr(100, 100, CV_8UC3, cv::Scalar(255, 0, 0)); // Blue image
    cv::Mat gray = ImageOps::toGray(bgr);
    assert(gray.channels() == 1);
    assert(gray.rows == 100 && gray.cols == 100);
    std::cout << "testToGray PASSED\n";
}

#include "../src/engine/cv/Matcher.hpp"

void testExtractInnerSquare() {
    // Create a 500x500 dummy image
    cv::Mat bgr(500, 500, CV_8UC3, cv::Scalar(255, 255, 255));
    // Draw a dark inner square
    cv::rectangle(bgr, cv::Rect(50, 50, 400, 400), cv::Scalar(0, 0, 0), cv::FILLED);
    
    cv::Mat extracted = Extractor::extractInnerSquare(bgr);
    
    // We expect it to crop the inner square roughly.
    assert(!extracted.empty());
    std::cout << "testExtractInnerSquare PASSED\n";
}

void testMatcher() {
    // Dummy global map
    cv::Mat mapBgr(1000, 1000, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::rectangle(mapBgr, cv::Rect(500, 500, 100, 100), cv::Scalar(255, 255, 255), 2);
    
    // Dummy patch
    cv::Mat patchBgr(100, 100, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::rectangle(patchBgr, cv::Rect(0, 0, 100, 100), cv::Scalar(255, 255, 255), 2);
    
    std::vector<float> scales = {1.0f};
    std::vector<float> angles = {0.0f};
    auto results = Matcher::matchPatchMultiscaleTopK(mapBgr, patchBgr, scales, angles, 40, 120, 5);
    
    assert(!results.empty());
    assert(results[0].score > 0.0);
    // Should roughly match near 500,500
    assert(std::abs(results[0].topLeft.x - 500) < 5);
    assert(std::abs(results[0].topLeft.y - 500) < 5);
    std::cout << "testMatcher PASSED\n";
}

int main() {
    std::cout << "Running CV Tests...\n";
    testToGray();
    testExtractInnerSquare();
    testMatcher();
    std::cout << "All tests passed!\n";
    return 0;
}
