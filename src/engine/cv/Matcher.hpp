#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

namespace treasure::cv {

struct MatchResult {
    float score;
    float scale;
    float angle;
    ::cv::Point topLeft;
    ::cv::Size patchSize;
    ::cv::Point2f centerPx;
};

class Matcher {
public:
    static ::cv::Mat estimateWaterMask(const ::cv::Mat& mapBgr);

    static std::vector<MatchResult> matchPatchMultiscaleTopK(
        const ::cv::Mat& mapBgr,
        const ::cv::Mat& patchBgr,
        const std::vector<float>& scales,
        const std::vector<float>& angles,
        int canny1,
        int canny2,
        int blurKsize,
        float wEdges = 0.7f,
        float wGray = 0.3f,
        int topK = 7,
        ::cv::Rect2i* roi = nullptr
    );

private:
    static void prepRepr(const ::cv::Mat& bgr, int canny1, int canny2, int blurKsize, ::cv::Mat& edges, ::cv::Mat& ng);
    static std::vector<std::pair<float, ::cv::Point>> topKFromResponse(::cv::Mat& res, int k, int suppress = 25);
};

} // namespace treasure::cv
