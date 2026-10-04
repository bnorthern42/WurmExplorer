#include "MainWindow.hpp"
#include "../engine/ZoomMapCanvas.hpp"
#include "components/TopControlBar.hpp"
#include "../features/treasure/TreasurePanel.hpp"
#include "../engine/cv/Extractor.hpp"
#include "../engine/cv/Matcher.hpp"
#include "../engine/cv/Ocr.hpp"

#include <QMessageBox>
#include <QtConcurrent>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/imgcodecs.hpp>
#include <algorithm>

void MainWindow::onLocateRequested(const QString& screenshotPath) {
    ::cv::Mat mapMat = mapCanvas->getMapMat();
    if (mapMat.empty()) {
        QMessageBox::warning(this, "Error", "No map loaded on canvas.");
        return;
    }
    
    ::cv::Mat screenshotBgr = ::cv::imread(screenshotPath.toStdString());
    if (screenshotBgr.empty()) {
        QMessageBox::warning(this, "Error", "Failed to load screenshot.");
        return;
    }

    std::vector<float> scales = {0.18f, 0.19f, 0.20f, 0.21f, 0.22f};
    
    int mapQl = treasurePanel->getQl();
    std::vector<float> angles = {0.0f};
    
    if (mapQl > 80) {
        float stdDev = 25.0f - (100.0f - mapQl);
        float maxAngle = stdDev * 3.0f;
        
        angles.clear();
        for (float a = -maxAngle; a <= maxAngle; a += 10.0f) {
            angles.push_back(a);
        }
    }

    int canny1 = treasurePanel->getCanny1();
    int canny2 = treasurePanel->getCanny2();
    int blurSize = treasurePanel->getBlurSize();
    float edgeW = treasurePanel->getEdgeWeight();
    float grayW = treasurePanel->getGrayWeight();

    TreasurePanel* tp = treasurePanel;
    tp->clearDebug();
    
    QString currentServer = currentConfig ? QString::fromStdString(currentConfig->name) : "Unknown";
    QString currentLayer = topBar->currentMapType();
    tp->appendDebug(QString("Target: Scanning %1 (%2)").arg(currentServer).arg(currentLayer));
    tp->appendDebug("Starting extraction and matching...");
    
    ::cv::Rect2i manualRoi;
    bool hasManualRoi = mapCanvas->getRoi(manualRoi);
    if (hasManualRoi) {
        tp->appendDebug("Using manual ROI for search.");
    }

    QFuture<std::vector<treasure::cv::MatchResult>> future = QtConcurrent::run([screenshotBgr, mapMat, scales, angles, canny1, canny2, blurSize, edgeW, grayW, tp, manualRoi, hasManualRoi]() -> std::vector<treasure::cv::MatchResult> {
        ::cv::Mat patch = treasure::cv::Extractor::extractMapSquareFromScreenshot(screenshotBgr);
        
        treasure::cv::Extractor::XDetectResult xResult;
        bool hasX = treasure::cv::Extractor::detectXCenter(patch, xResult);
        
        ::cv::Mat hsv, maskRed1, maskRed2, maskDark, maskToInpaint;
        ::cv::cvtColor(patch, hsv, ::cv::COLOR_BGR2HSV);
        
        ::cv::inRange(hsv, ::cv::Scalar(0, 70, 50), ::cv::Scalar(10, 255, 255), maskRed1);
        ::cv::inRange(hsv, ::cv::Scalar(170, 70, 50), ::cv::Scalar(180, 255, 255), maskRed2);
        ::cv::inRange(hsv, ::cv::Scalar(0, 0, 0), ::cv::Scalar(180, 255, 60), maskDark);
        
        ::cv::bitwise_or(maskRed1, maskRed2, maskToInpaint);
        ::cv::bitwise_or(maskToInpaint, maskDark, maskToInpaint);
        
        ::cv::Mat restrictMask = ::cv::Mat::zeros(patch.size(), CV_8U);
        int ph = patch.rows;
        int pw = patch.cols;
        
        ::cv::Rect textRect(0, static_cast<int>(ph * 0.75), pw, ph - static_cast<int>(ph * 0.75));
        restrictMask(textRect).setTo(::cv::Scalar(255));
        
        if (hasX) {
            int xs = static_cast<int>(xResult.size * 1.5);
            int x0 = std::max(0, static_cast<int>(xResult.cx) - xs / 2);
            int y0 = std::max(0, static_cast<int>(xResult.cy) - xs / 2);
            int rw = std::min(pw - x0, xs);
            int rh = std::min(ph - y0, xs);
            ::cv::Rect xRect(x0, y0, rw, rh);
            restrictMask(xRect).setTo(::cv::Scalar(255));
        }
        
        ::cv::bitwise_and(maskToInpaint, restrictMask, maskToInpaint);
        
        ::cv::Mat kernel = ::cv::getStructuringElement(::cv::MORPH_RECT, ::cv::Size(5, 5));
        ::cv::dilate(maskToInpaint, maskToInpaint, kernel, ::cv::Point(-1, -1), 2);
        ::cv::inpaint(patch, maskToInpaint, patch, 5, ::cv::INPAINT_TELEA);
        
        std::string ocrText = treasure::cv::Ocr::extractProspectText(screenshotBgr);
        
        QMetaObject::invokeMethod(tp, [tp, ocrText]() {
            tp->appendDebug("OCR Extracted Text:\n" + QString::fromStdString(ocrText));
        }, Qt::QueuedConnection);

        auto deedOpt = treasure::cv::Ocr::findDeedCoordinates(ocrText);
        
        ::cv::Rect2i* roiPtr = nullptr;
        ::cv::Rect2i roi;
        if (hasManualRoi) {
            roi = manualRoi;
            roiPtr = &roi;
        } else if (deedOpt.has_value()) {
            int cx = deedOpt.value().x;
            int cy = deedOpt.value().y;
            roi = ::cv::Rect2i(cx - 250, cy - 250, 500, 500);
            roiPtr = &roi;
        }

        auto results = treasure::cv::Matcher::matchPatchMultiscaleTopK(mapMat, patch, scales, angles, canny1, canny2, blurSize, edgeW, grayW, 7, roiPtr);
        
        if (hasX) {
            for (auto& res : results) {
                res.centerPx.x = res.topLeft.x + (xResult.cx * res.scale);
                res.centerPx.y = res.topLeft.y + (xResult.cy * res.scale);
            }
        }
        
        return results;
    });
    
    watcher.setFuture(future);
    mapCanvas->clearMarkers();
}

void MainWindow::onLocateFinished() {
    std::vector<treasure::cv::MatchResult> results = watcher.result();
    m_lastResults = results;
    
    if (results.empty()) {
        QMessageBox::information(this, "Result", "No matches found.");
        treasurePanel->appendDebug("Finished: No matches found.");
        treasurePanel->setMatchResults(results);
    } else {
        treasurePanel->setMatchResults(results);
        mapCanvas->setMarkers(results);
        
        float bestX = results[0].centerPx.x;
        float bestY = results[0].centerPx.y;
        mapCanvas->centerOn(bestX, bestY);
        
        treasurePanel->appendDebug(QString("Finished! Best Match Score: %1%").arg(static_cast<int>(results[0].score * 100)));
    }
}

void MainWindow::onMatchSelected(int index) {
    if (index >= 0 && index < static_cast<int>(m_lastResults.size())) {
        float cx = m_lastResults[index].centerPx.x;
        float cy = m_lastResults[index].centerPx.y;
        mapCanvas->centerOn(cx, cy);
    }
}
