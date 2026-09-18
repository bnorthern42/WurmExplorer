#include "Extractor.hpp"
#include "ImageOps.hpp"
#include <algorithm>
#include <vector>

namespace treasure::cv {

::cv::Mat Extractor::edgeIntegral(const ::cv::Mat& gray, int c1, int c2) {
    ::cv::Mat edges = ImageOps::cannyEdges(gray, c1, c2, 5);
    ::cv::Mat integ;
    ::cv::integral(edges, integ, CV_32S);
    return integ;
}

int Extractor::rectSum(const ::cv::Mat& integ, int x, int y, int s) {
    int x2 = x + s;
    int y2 = y + s;
    return integ.at<int>(y2, x2) - integ.at<int>(y, x2) - integ.at<int>(y2, x) + integ.at<int>(y, x);
}

::cv::Mat Extractor::trimParchmentBorder(const ::cv::Mat& squareBgr) {
    if (squareBgr.empty()) return squareBgr;
    int h = squareBgr.rows;
    int w = squareBgr.cols;
    int m = std::max(6, static_cast<int>(std::round(0.06 * std::min(h, w))));
    
    ::cv::Mat lab;
    ::cv::cvtColor(squareBgr, lab, ::cv::COLOR_BGR2Lab);
    lab.convertTo(lab, CV_16S); // To avoid overflow when computing distance

    // Just an approximation for median: compute average of border pixels instead
    // since exact median of 3D vectors is expensive. Mean works fine for parchment.
    long long L_sum = 0, a_sum = 0, b_sum = 0;
    int count = 0;
    
    auto addPixels = [&](int y0, int y1, int x0, int x1) {
        for (int y = y0; y < y1; ++y) {
            for (int x = x0; x < x1; ++x) {
                ::cv::Vec3s p = lab.at<::cv::Vec3s>(y, x);
                L_sum += p[0];
                a_sum += p[1];
                b_sum += p[2];
                count++;
            }
        }
    };
    
    addPixels(0, m, 0, w);
    addPixels(h - m, h, 0, w);
    addPixels(m, h - m, 0, m);
    addPixels(m, h - m, w - m, w);
    
    if (count == 0) return squareBgr.clone();
    
    ::cv::Vec3s med(L_sum / count, a_sum / count, b_sum / count);
    
    ::cv::Mat mask(h, w, CV_8U, ::cv::Scalar(0));
    float thr = 18.0f;
    int minX = w, maxX = 0;
    int minY = h, maxY = 0;
    int validPixels = 0;
    
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            ::cv::Vec3s p = lab.at<::cv::Vec3s>(y, x);
            float d = std::sqrt(std::pow(p[0] - med[0], 2) + std::pow(p[1] - med[1], 2) + std::pow(p[2] - med[2], 2));
            if (d > thr) {
                mask.at<uchar>(y, x) = 255;
            }
        }
    }
    
    int k = std::max(3, (m / 2) | 1);
    ::cv::Mat element = ::cv::getStructuringElement(::cv::MORPH_ELLIPSE, ::cv::Size(k, k));
    ::cv::morphologyEx(mask, mask, ::cv::MORPH_CLOSE, element);
    ::cv::medianBlur(mask, mask, 5);
    
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (mask.at<uchar>(y, x) > 0) {
                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
                validPixels++;
            }
        }
    }
    
    if (validPixels < 1000) return squareBgr.clone(); // Give up
    
    int pad = std::max(2, static_cast<int>(std::round(0.01 * std::min(h, w))));
    minX = std::max(0, minX - pad);
    minY = std::max(0, minY - pad);
    maxX = std::min(w - 1, maxX + pad);
    maxY = std::min(h - 1, maxY + pad);
    
    ::cv::Rect cropRegion(minX, minY, maxX - minX + 1, maxY - minY + 1);
    return squareBgr(cropRegion).clone();
}

::cv::Mat Extractor::extractInnerSquare(const ::cv::Mat& bgr) {
    if (bgr.empty()) return bgr;
    int h = bgr.rows;
    int w = bgr.cols;
    ::cv::Mat gray = ImageOps::toGray(bgr);
    ::cv::Mat integ = edgeIntegral(gray);
    
    int minDim = std::min(h, w);
    std::vector<float> sizeFracs = {0.70f, 0.75f, 0.80f, 0.85f, 0.90f, 0.95f};
    
    int bestScore = -1;
    int bestX = 0, bestY = 0, bestS = 0;
    
    for (float frac : sizeFracs) {
        int s = std::clamp(static_cast<int>(frac * minDim), 64, minDim);
        int cx = w / 2;
        int cy = h / 2;
        int span = static_cast<int>(0.20 * minDim);
        int step = std::max(2, s / 80);
        
        int x0 = std::max(0, cx - s / 2 - span);
        int x1 = std::min(w - s, cx - s / 2 + span);
        int y0 = std::max(0, cy - s / 2 - span);
        int y1 = std::min(h - s, cy - s / 2 + span);
        
        for (int y = y0; y <= y1; y += step) {
            for (int x = x0; x <= x1; x += step) {
                int score = rectSum(integ, x, y, s);
                if (score > bestScore) {
                    bestScore = score;
                    bestX = x;
                    bestY = y;
                    bestS = s;
                }
            }
        }
    }
    
    if (bestScore == -1) {
        bestS = static_cast<int>(0.85 * minDim);
        bestX = (w - bestS) / 2;
        bestY = (h - bestS) / 2;
    }
    
    ::cv::Rect squareRect(bestX, bestY, bestS, bestS);
    ::cv::Mat square = bgr(squareRect).clone();
    
    return trimParchmentBorder(square);
}

::cv::Mat Extractor::extractMapSquareFromScreenshot(const ::cv::Mat& bgr, int outSize) {
    ::cv::Mat trimmed = extractInnerSquare(bgr);
    return ImageOps::resizeKeep(trimmed, outSize, outSize);
}

bool Extractor::detectXCenter(const ::cv::Mat& map500Bgr, XDetectResult& outResult) {
    if (map500Bgr.empty()) return false;
    
    ::cv::Mat hsv, maskRed1, maskRed2, maskDark, inv;
    ::cv::cvtColor(map500Bgr, hsv, ::cv::COLOR_BGR2HSV);
    
    // Wurm treasure X is typically red, but can be black/dark brown depending on settings/map.
    ::cv::inRange(hsv, ::cv::Scalar(0, 70, 50), ::cv::Scalar(10, 255, 255), maskRed1);
    ::cv::inRange(hsv, ::cv::Scalar(170, 70, 50), ::cv::Scalar(180, 255, 255), maskRed2);
    ::cv::inRange(hsv, ::cv::Scalar(0, 0, 0), ::cv::Scalar(180, 255, 60), maskDark);
    
    ::cv::bitwise_or(maskRed1, maskRed2, inv);
    ::cv::bitwise_or(inv, maskDark, inv);
    
    int H = inv.rows;
    int W = inv.cols;
    int maskCompassPx = 120;
    int borderPx = 8;
    
    ::cv::Mat inv2 = inv.clone();
    
    if (H >= maskCompassPx && W >= maskCompassPx) {
        ::cv::Rect compassRect(W - maskCompassPx, H - maskCompassPx, maskCompassPx, maskCompassPx);
        inv2(compassRect).setTo(::cv::Scalar(0));
    }
    
    if (H > 2*borderPx && W > 2*borderPx) {
        inv2(::cv::Rect(0, 0, W, borderPx)).setTo(::cv::Scalar(0));
        inv2(::cv::Rect(0, H - borderPx, W, borderPx)).setTo(::cv::Scalar(0));
        inv2(::cv::Rect(0, 0, borderPx, H)).setTo(::cv::Scalar(0));
        inv2(::cv::Rect(W - borderPx, 0, borderPx, H)).setTo(::cv::Scalar(0));
    }
    
    std::vector<float> templateFracs = {0.08f, 0.10f, 0.12f, 0.14f};
    bool found = false;
    float bestScore = -1.0f;
    int bestX = 0, bestY = 0, bestS = 0;
    
    for (float frac : templateFracs) {
        int s = std::clamp(static_cast<int>(frac * 500), 18, 120);
        int thick = std::max(2, s / 10);
        
        ::cv::Mat templ = ::cv::Mat::zeros(s, s, CV_8U);
        ::cv::line(templ, ::cv::Point(0, 0), ::cv::Point(s - 1, s - 1), ::cv::Scalar(255), thick);
        ::cv::line(templ, ::cv::Point(0, s - 1), ::cv::Point(s - 1, 0), ::cv::Scalar(255), thick);
        ::cv::GaussianBlur(templ, templ, ::cv::Size(5, 5), 0);
        
        if (inv2.rows < templ.rows || inv2.cols < templ.cols) continue;
        
        ::cv::Mat res;
        ::cv::matchTemplate(inv2, templ, res, ::cv::TM_CCOEFF_NORMED);
        
        double minVal, maxVal;
        ::cv::Point minLoc, maxLoc;
        ::cv::minMaxLoc(res, &minVal, &maxVal, &minLoc, &maxLoc);
        
        if (maxVal > bestScore) {
            bestScore = static_cast<float>(maxVal);
            bestX = maxLoc.x;
            bestY = maxLoc.y;
            bestS = s;
            found = true;
        }
    }
    
    if (!found) return false;
    
    outResult.cx = bestX + bestS / 2.0f;
    outResult.cy = bestY + bestS / 2.0f;
    outResult.size = bestS;
    outResult.score = bestScore;
    return true;
}

} // namespace treasure::cv
