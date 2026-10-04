#pragma once

#pragma push_macro("signals")
#undef signals
#include <opencv2/opencv.hpp>
#pragma pop_macro("signals")

namespace treasure::cv {

class ImageOps {
public:
    static ::cv::Mat toGray(const ::cv::Mat& bgr);
    static ::cv::Mat cannyEdges(const ::cv::Mat& gray, int c1, int c2, int blurKsize = 5);
    static ::cv::Mat normGray(const ::cv::Mat& gray);
    static ::cv::Mat resizeKeep(const ::cv::Mat& img, int w, int h, int interp = ::cv::INTER_AREA);
};

} // namespace treasure::cv
