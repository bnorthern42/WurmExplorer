#pragma once

#pragma push_macro("signals")
#undef signals
#include <opencv2/opencv.hpp>
#pragma pop_macro("signals")
#include <string>
#include <optional>

namespace treasure::cv {

class Ocr {
public:
    // Extract text from the bottom 20% of the screenshot
    static std::string extractProspectText(const ::cv::Mat& screenshotBgr);
    
    // Look up the coordinates of a deed if mentioned in the text
    static std::optional<::cv::Point> findDeedCoordinates(const std::string& text);
};

} // namespace treasure::cv
