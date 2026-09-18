#include "Ocr.hpp"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <map>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace treasure::cv {

std::string Ocr::extractProspectText(const ::cv::Mat& screenshotBgr) {
    if (screenshotBgr.empty()) return "";

    // The prospect text is usually in the event window at the bottom of the screen.
    // We'll crop the bottom 25% of the screen.
    int cropY = static_cast<int>(screenshotBgr.rows * 0.75);
    int cropH = screenshotBgr.rows - cropY;
    ::cv::Rect roi(0, cropY, screenshotBgr.cols, cropH);
    ::cv::Mat textImg = screenshotBgr(roi).clone();
    
    // Convert to grayscale for OCR
    ::cv::Mat gray;
    ::cv::cvtColor(textImg, gray, ::cv::COLOR_BGR2GRAY);

    tesseract::TessBaseAPI tess;
    if (tess.Init(nullptr, "eng", tesseract::OEM_LSTM_ONLY) != 0) {
        std::cerr << "Failed to initialize Tesseract." << std::endl;
        return "";
    }
    
    tess.SetImage(gray.data, gray.cols, gray.rows, 1, gray.step);
    
    char* textPtr = tess.GetUTF8Text();
    std::string text = "";
    if (textPtr) {
        text = std::string(textPtr);
        delete[] textPtr;
    }
    
    tess.End();
    return text;
}

std::optional<::cv::Point> Ocr::findDeedCoordinates(const std::string& text) {
    if (text.empty()) return std::nullopt;
    
    std::string lowerText = text;
    std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(),
                   [](unsigned char c){ return std::tolower(c); });
                   
    // Mock deed database mapping lowercased deed names to map coordinates
    static const std::map<std::string, ::cv::Point> MOCK_DEEDS = {
        {"newtown", ::cv::Point(2000, 2500)},
        {"oldtown", ::cv::Point(4000, 4500)},
        {"cliffside", ::cv::Point(3500, 1500)},
        {"valley forge", ::cv::Point(1500, 6000)}
    };
    
    for (const auto& [deedName, pt] : MOCK_DEEDS) {
        if (lowerText.find(deedName) != std::string::npos) {
            std::cout << "OCR matched deed: " << deedName << " at (" << pt.x << "," << pt.y << ")" << std::endl;
            return pt;
        }
    }
    
    return std::nullopt;
}

} // namespace treasure::cv
