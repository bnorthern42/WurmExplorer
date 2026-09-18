#pragma once

#include <opencv2/opencv.hpp>
#include <map>
#include <string>

namespace treasure::cv {

class Extractor {
public:
    // Extract inner map square, trimming the parchment border
    static ::cv::Mat extractInnerSquare(const ::cv::Mat& bgr);
    
    // Extract map square and resize it to out_size
    static ::cv::Mat extractMapSquareFromScreenshot(const ::cv::Mat& bgr, int outSize = 500);

    struct XDetectResult {
        float cx, cy;
        int size;
        float score;
    };

    // Detect the red X in the 500x500 map square
    static bool detectXCenter(const ::cv::Mat& map500Bgr, XDetectResult& outResult);

private:
    static ::cv::Mat edgeIntegral(const ::cv::Mat& gray, int c1 = 40, int c2 = 120);
    static int rectSum(const ::cv::Mat& integ, int x, int y, int s);
    static ::cv::Mat trimParchmentBorder(const ::cv::Mat& squareBgr);
};

} // namespace treasure::cv
