#include "ImageOps.hpp"
#include <stdexcept>

namespace treasure::cv {

::cv::Mat ImageOps::toGray(const ::cv::Mat& bgr) {
    if (bgr.empty()) return ::cv::Mat();
    ::cv::Mat gray;
    ::cv::cvtColor(bgr, gray, ::cv::COLOR_BGR2GRAY);
    return gray;
}

::cv::Mat ImageOps::cannyEdges(const ::cv::Mat& gray, int c1, int c2, int blurKsize) {
    if (gray.empty()) return ::cv::Mat();
    ::cv::Mat edges = gray.clone();
    if (blurKsize >= 3) {
        ::cv::GaussianBlur(gray, edges, ::cv::Size(blurKsize, blurKsize), 0);
    }
    ::cv::Canny(edges, edges, c1, c2);
    return edges;
}

::cv::Mat ImageOps::normGray(const ::cv::Mat& gray) {
    if (gray.empty()) return ::cv::Mat();
    ::cv::Mat g;
    gray.convertTo(g, CV_32F);
    
    ::cv::Scalar mean, stddev;
    ::cv::meanStdDev(g, mean, stddev);
    
    g = (g - mean[0]) / (stddev[0] + 1e-6);
    ::cv::normalize(g, g, 0, 255, ::cv::NORM_MINMAX);
    
    ::cv::Mat out;
    g.convertTo(out, CV_8U);
    return out;
}

::cv::Mat ImageOps::resizeKeep(const ::cv::Mat& img, int w, int h, int interp) {
    if (img.empty()) return ::cv::Mat();
    ::cv::Mat out;
    ::cv::resize(img, out, ::cv::Size(w, h), 0, 0, interp);
    return out;
}

} // namespace treasure::cv
