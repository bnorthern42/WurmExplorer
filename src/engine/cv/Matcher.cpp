#include "Matcher.hpp"
#include "ImageOps.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace treasure::cv {

void Matcher::prepRepr(const ::cv::Mat& bgr, int canny1, int canny2, int blurKsize, ::cv::Mat& edges, ::cv::Mat& ng) {
    ::cv::Mat gray = ImageOps::toGray(bgr);
    edges = ImageOps::cannyEdges(gray, canny1, canny2, blurKsize);
    // Use color instead of normalized grayscale to allow 3-channel CCOEFF_NORMED matching
    // This perfectly matches features like blue water and black peat!
    ng = bgr.clone();
    if (blurKsize >= 3) {
        ::cv::GaussianBlur(ng, ng, ::cv::Size(blurKsize, blurKsize), 0);
    }
}

::cv::Mat Matcher::estimateWaterMask(const ::cv::Mat& mapBgr) {
    ::cv::Mat hsv;
    ::cv::cvtColor(mapBgr, hsv, ::cv::COLOR_BGR2HSV);
    
    // Default water bounds (from python)
    ::cv::Scalar lower(90, 40, 30);
    ::cv::Scalar upper(140, 255, 255);
    
    ::cv::Mat mask;
    ::cv::inRange(hsv, lower, upper, mask);
    ::cv::medianBlur(mask, mask, 5);
    return mask;
}

std::vector<std::pair<float, ::cv::Point>> Matcher::topKFromResponse(::cv::Mat& res, int k, int suppress) {
    std::vector<std::pair<float, ::cv::Point>> out;
    for (int i = 0; i < k; ++i) {
        double minVal, maxVal;
        ::cv::Point minLoc, maxLoc;
        ::cv::minMaxLoc(res, &minVal, &maxVal, &minLoc, &maxLoc);
        
        if (!std::isfinite(maxVal) || maxVal <= -1e9) break;
        
        out.push_back({static_cast<float>(maxVal), maxLoc});
        
        int x0 = std::max(0, maxLoc.x - suppress);
        int y0 = std::max(0, maxLoc.y - suppress);
        int x1 = std::min(res.cols, maxLoc.x + suppress);
        int y1 = std::min(res.rows, maxLoc.y + suppress);
        
        for (int y = y0; y < y1; ++y) {
            for (int x = x0; x < x1; ++x) {
                res.at<float>(y, x) = -1e9f;
            }
        }
    }
    return out;
}

std::vector<MatchResult> Matcher::matchPatchMultiscaleTopK(
    const ::cv::Mat& mapBgr, const ::cv::Mat& patchBgr,
    const std::vector<float>& scales, const std::vector<float>& angles,
    int canny1, int canny2, int blurKsize,
    float wEdges, float wGray, int topK, ::cv::Rect2i* roi) 
{
    int xoff = 0, yoff = 0;
    ::cv::Mat mapWork = mapBgr;
    
    if (roi != nullptr) {
        int x0 = std::max(0, std::min(mapBgr.cols - 1, roi->x));
        int y0 = std::max(0, std::min(mapBgr.rows - 1, roi->y));
        int x1 = std::max(1, std::min(mapBgr.cols, roi->x + roi->width));
        int y1 = std::max(1, std::min(mapBgr.rows, roi->y + roi->height));
        
        xoff = x0;
        yoff = y0;
        ::cv::Rect validRoi(xoff, yoff, x1 - x0, y1 - y0);
        mapWork = mapBgr(validRoi).clone();
    }
    
    ::cv::Mat mapEdges, mapNg;
    prepRepr(mapWork, canny1, canny2, blurKsize, mapEdges, mapNg);
    
    // Chamfer distance transform
    float maxDist = 15.0f;
    ::cv::Mat invMapEdges;
    ::cv::bitwise_not(mapEdges, invMapEdges);
    
    ::cv::Mat dist;
    ::cv::distanceTransform(invMapEdges, dist, ::cv::DIST_L2, 3);
    
    ::cv::Mat distScore = ::cv::Mat::zeros(dist.size(), CV_32F);
    for (int r = 0; r < dist.rows; ++r) {
        for (int c = 0; c < dist.cols; ++c) {
            float d = dist.at<float>(r, c);
            float score = std::max(0.0f, maxDist - d) / maxDist;
            distScore.at<float>(r, c) = score;
        }
    }
    
    std::vector<MatchResult> candidates;
    
    #pragma omp parallel for collapse(2)
    for (size_t i = 0; i < scales.size(); ++i) {
        for (size_t j = 0; j < angles.size(); ++j) {
            float s = scales[i];
            float angle = angles[j];
            int pw = std::max(32, static_cast<int>(std::round(patchBgr.cols * s)));
            int ph = std::max(32, static_cast<int>(std::round(patchBgr.rows * s)));
            
            if (ph >= mapEdges.rows || pw >= mapEdges.cols) continue;
            
            ::cv::Mat patchRs;
            ::cv::resize(patchBgr, patchRs, ::cv::Size(pw, ph), 0, 0, ::cv::INTER_AREA);
            
            // Rotate the resized template
            if (std::abs(angle) > 1.0f) {
                ::cv::Point2f center(patchRs.cols / 2.0f, patchRs.rows / 2.0f);
                ::cv::Mat rot = ::cv::getRotationMatrix2D(center, angle, 1.0);
                
                ::cv::Rect2f bbox = ::cv::RotatedRect(center, patchRs.size(), angle).boundingRect2f();
                rot.at<double>(0, 2) += bbox.width / 2.0 - patchRs.cols / 2.0;
                rot.at<double>(1, 2) += bbox.height / 2.0 - patchRs.rows / 2.0;
                
                ::cv::Mat rotated;
                // Use BORDER_CONSTANT with mean parchment color? No, we can just mask it out.
                // Wait, we need to mask the corners that were added by rotation.
                // The edge integral doesn't matter, but wGray template matching does.
                // If we don't mask wGray, the black corners will fail the normalized cross correlation.
                // We'll use 0 as a magic number or create a mask. For now, since wGray is CCOEFF, it's better to use mask but TM_CCOEFF_NORMED with mask is not universally supported.
                // We will just rotate and let it be, or we can use TM_CCORR_NORMED which supports masks.
                // Actually, TM_CCORR_NORMED does not handle mean shifting.
                // Let's just use BORDER_REPLICATE so the corners are filled with edge colors.
                ::cv::warpAffine(patchRs, rotated, rot, bbox.size(), ::cv::INTER_LINEAR, ::cv::BORDER_REPLICATE);
                patchRs = rotated;
                pw = patchRs.cols;
                ph = patchRs.rows;
            }
            
            ::cv::Mat pEdges, pNg;
            prepRepr(patchRs, canny1, canny2, blurKsize, pEdges, pNg);
            
            ::cv::Mat pMask(pEdges.size(), CV_32F, ::cv::Scalar(0));
            int edgePixels = 0;
            for (int r = 0; r < pEdges.rows; ++r) {
                for (int c = 0; c < pEdges.cols; ++c) {
                    if (pEdges.at<uchar>(r, c) > 0) {
                        pMask.at<float>(r, c) = 1.0f;
                        edgePixels++;
                    }
                }
            }
            
            if (wEdges > 0 && edgePixels < 10) continue;
            
            ::cv::Mat finalRes;
            
            if (wEdges > 0) {
                ::cv::Mat resE;
                ::cv::matchTemplate(distScore, pMask, resE, ::cv::TM_CCORR);
                resE /= static_cast<float>(edgePixels);
                finalRes = resE * wEdges;
            }
            
            if (wGray > 0) {
                ::cv::Mat resG;
                ::cv::matchTemplate(mapNg, pNg, resG, ::cv::TM_CCOEFF_NORMED);
                
                if (finalRes.empty()) {
                    finalRes = resG * wGray;
                } else {
                    int minH = std::min(finalRes.rows, resG.rows);
                    int minW = std::min(finalRes.cols, resG.cols);
                    
                    ::cv::Rect roiCrop(0, 0, minW, minH);
                    ::cv::Mat finalCrop = finalRes(roiCrop);
                    ::cv::Mat resGCrop = resG(roiCrop);
                    
                    finalRes = finalCrop + resGCrop * wGray;
                }
            }
            
            if (finalRes.empty()) continue;
            
            int suppress = std::max(10, static_cast<int>(std::round(std::min(pw, ph) * 0.15f)));
            auto topkList = topKFromResponse(finalRes, topK, suppress);
            
            for (const auto& pair : topkList) {
                float score = pair.first;
                float weightSum = wEdges + wGray;
                if (weightSum > 0.0f) {
                    score /= weightSum;
                }
                score = std::max(0.0f, score); // Prevent negative scores from CCOEFF_NORMED
                
                ::cv::Point pt = pair.second;
                ::cv::Point2f center(pt.x + pw / 2.0f + xoff, pt.y + ph / 2.0f + yoff);
                
                #pragma omp critical
                {
                    candidates.push_back({
                        score,
                        s,
                        angle,
                        ::cv::Point(pt.x + xoff, pt.y + yoff),
                        ::cv::Size(pw, ph),
                        center
                    });
                }
            }
        }
    }
    
    std::sort(candidates.begin(), candidates.end(), [](const MatchResult& a, const MatchResult& b) {
        return a.score > b.score;
    });
    
    std::vector<MatchResult> filtered;
    for (const auto& best : candidates) {
        bool conflict = false;
        for (const auto& f : filtered) {
            int dx = best.topLeft.x - f.topLeft.x;
            int dy = best.topLeft.y - f.topLeft.y;
            if (dx*dx + dy*dy < 100*100) {
                conflict = true;
                break;
            }
        }
        if (!conflict) {
            filtered.push_back(best);
        }
        if (filtered.size() >= static_cast<size_t>(topK)) {
            break;
        }
    }
    
    return filtered;
}

} // namespace treasure::cv
