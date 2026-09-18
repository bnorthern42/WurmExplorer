#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QListWidget>
#include <vector>
#include "../../engine/cv/Matcher.hpp"

class TreasurePanel : public QWidget {
    Q_OBJECT

public:
    explicit TreasurePanel(QWidget *parent = nullptr);
    ~TreasurePanel() override = default;

    int getCanny1() const;
    int getCanny2() const;
    int getBlurSize() const;
    int getQl() const;
    float getEdgeWeight() const;
    float getGrayWeight() const;
    
    void clearDebug();
    void appendDebug(const QString& text);
    
    void setMatchResults(const std::vector<treasure::cv::MatchResult>& results);

signals:
    void locateRequested(const QString& screenshotPath);
    void matchSelected(int index);
    void setRoiModeRequested(bool active);

private slots:
    void onBrowseClicked();
    void onPasteClicked();
    void onLocateClicked();
    void onSetRoiClicked();
    void onClearRoiClicked();
    void onMatchListItemClicked(int row);

private:
    void setupUi();

    QLineEdit* screenshotPathEdit;
    QPushButton* browseButton;
    QPushButton* pasteButton;
    QPushButton* locateButton;
    
    QPushButton* setRoiButton;
    QPushButton* clearRoiButton;
    QListWidget* matchListView;

    QSpinBox* canny1Spin;
    QSpinBox* canny2Spin;
    QSpinBox* blurSpin;
    QSpinBox* qlSpin;
    QDoubleSpinBox* edgeWeightSpin;
    QDoubleSpinBox* grayWeightSpin;
    QTextEdit* debugArea;
    
    std::vector<treasure::cv::MatchResult> currentMatches;
};
