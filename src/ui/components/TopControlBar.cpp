#include "TopControlBar.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>

TopControlBar::TopControlBar(QWidget *parent) : QWidget(parent) {
    setupUi();

    connect(clusterCombo, &QComboBox::currentTextChanged, this, &TopControlBar::clusterChanged);
    connect(serverCombo, &QComboBox::currentTextChanged, this, &TopControlBar::serverChanged);
    connect(mapTypeCombo, &QComboBox::currentTextChanged, this, &TopControlBar::mapTypeChanged);
    connect(exitButton, &QPushButton::clicked, this, &TopControlBar::quitRequested);
}

void TopControlBar::setClusters(const std::vector<std::string>& clusters) {
    QString current = clusterCombo->currentText();
    clusterCombo->blockSignals(true);
    clusterCombo->clear();
    for (const auto& c : clusters) clusterCombo->addItem(QString::fromStdString(c));
    int idx = clusterCombo->findText(current);
    if (idx >= 0) clusterCombo->setCurrentIndex(idx);
    else if (clusterCombo->count() > 0) clusterCombo->setCurrentIndex(0);
    clusterCombo->blockSignals(false);
}

void TopControlBar::setServers(const std::vector<std::string>& servers) {
    QString current = serverCombo->currentText();
    serverCombo->blockSignals(true);
    serverCombo->clear();
    for (const auto& s : servers) serverCombo->addItem(QString::fromStdString(s));
    int idx = serverCombo->findText(current);
    if (idx >= 0) serverCombo->setCurrentIndex(idx);
    else if (serverCombo->count() > 0) serverCombo->setCurrentIndex(0);
    serverCombo->blockSignals(false);
}

void TopControlBar::setMapTypes(const std::vector<std::string>& mapTypes) {
    QString current = mapTypeCombo->currentText();
    mapTypeCombo->blockSignals(true);
    mapTypeCombo->clear();
    for (const auto& m : mapTypes) mapTypeCombo->addItem(QString::fromStdString(m));
    int idx = mapTypeCombo->findText(current);
    if (idx >= 0) mapTypeCombo->setCurrentIndex(idx);
    else if (mapTypeCombo->count() > 0) mapTypeCombo->setCurrentIndex(0);
    mapTypeCombo->blockSignals(false);
}

QString TopControlBar::currentCluster() const { return clusterCombo->currentText(); }
QString TopControlBar::currentServer() const { return serverCombo->currentText(); }
QString TopControlBar::currentMapType() const {
    return mapTypeCombo->currentText();
}

void TopControlBar::setCurrentMapType(const QString& mapType) {
    int idx = mapTypeCombo->findText(mapType);
    if (idx >= 0) {
        mapTypeCombo->setCurrentIndex(idx);
    }
}

void TopControlBar::setMapTypeEnabled(bool enabled) {
    mapTypeCombo->setEnabled(enabled);
}

void TopControlBar::blockAllSignals(bool block) {
    clusterCombo->blockSignals(block);
    serverCombo->blockSignals(block);
    mapTypeCombo->blockSignals(block);
}

void TopControlBar::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(16);

    // Styling to make it look like a distinct top bar
    setObjectName("TopControlBar");
    setStyleSheet("#TopControlBar { background-color: #1e1e2e; border-bottom: 1px solid #313244; }");

    auto* titleLabel = new QLabel("WurmExplorer", this);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #8aadf4;");
    layout->addWidget(titleLabel);

    layout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    auto* clusterLabel = new QLabel("Cluster:", this);
    clusterLabel->setStyleSheet("color: #a6adc8;");
    clusterCombo = new QComboBox(this);
    clusterCombo->setObjectName("clusterCombo");

    auto* serverLabel = new QLabel("Server:", this);
    serverLabel->setStyleSheet("color: #a6adc8;");
    serverCombo = new QComboBox(this);
    serverCombo->setObjectName("serverCombo");

    auto* mapTypeLabel = new QLabel("Map Type:", this);
    mapTypeLabel->setStyleSheet("color: #a6adc8;");
    mapTypeCombo = new QComboBox(this);
    mapTypeCombo->setObjectName("mapTypeCombo");

    layout->addWidget(clusterLabel);
    layout->addWidget(clusterCombo);
    layout->addWidget(serverLabel);
    layout->addWidget(serverCombo);
    layout->addWidget(mapTypeLabel);
    layout->addWidget(mapTypeCombo);

    exitButton = new QPushButton("Exit", this);
    exitButton->setStyleSheet("color: #ed8796; font-weight: bold; border: none;");
    layout->addWidget(exitButton);
}
