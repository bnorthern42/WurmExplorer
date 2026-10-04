#pragma once

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class TopControlBar : public QWidget {
    Q_OBJECT

public:
    explicit TopControlBar(QWidget *parent = nullptr);
    ~TopControlBar() override = default;
    
    void setClusters(const std::vector<std::string>& clusters);
    void setServers(const std::vector<std::string>& servers);
    void setMapTypes(const std::vector<std::string>& mapTypes);
    
    QString currentCluster() const;
    QString currentServer() const;
    QString currentMapType() const;
    void setCurrentMapType(const QString& mapType);
    void setMapTypeEnabled(bool enabled);
    void setMapControlsVisible(bool visible);
    void setContextBreadcrumb(const QString& domain, const QString& title);
    
    void blockAllSignals(bool block);
    
signals:
    void clusterChanged(const QString& cluster);
    void serverChanged(const QString& server);
    void mapTypeChanged(const QString& mapType);
    void settingsRequested();
    void quitRequested();

private:
    void setupUi();

    QLabel* breadcrumbDomainLabel = nullptr;
    QLabel* breadcrumbTitleLabel = nullptr;
    QLabel* clusterLabel = nullptr;
    QComboBox* clusterCombo = nullptr;
    QLabel* serverLabel = nullptr;
    QComboBox* serverCombo = nullptr;
    QLabel* mapTypeLabel = nullptr;
    QComboBox* mapTypeCombo = nullptr;
    QPushButton* settingsButton = nullptr;
    QPushButton* exitButton = nullptr;
};

