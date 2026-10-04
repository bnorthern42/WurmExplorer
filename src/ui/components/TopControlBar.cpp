#include "TopControlBar.hpp"
#include "../ThemeTokens.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>

TopControlBar::TopControlBar(QWidget *parent) : QWidget(parent) {
    setupUi();

    connect(clusterCombo, &QComboBox::currentTextChanged, this, &TopControlBar::clusterChanged);
    connect(serverCombo, &QComboBox::currentTextChanged, this, &TopControlBar::serverChanged);
    connect(mapTypeCombo, &QComboBox::currentTextChanged, this, &TopControlBar::mapTypeChanged);
    connect(settingsButton, &QPushButton::clicked, this, &TopControlBar::settingsRequested);
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

void TopControlBar::setMapControlsVisible(bool visible) {
    if (mapTypeLabel) mapTypeLabel->setVisible(visible);
    if (mapTypeCombo) mapTypeCombo->setVisible(visible);
}

void TopControlBar::blockAllSignals(bool block) {
    clusterCombo->blockSignals(block);
    serverCombo->blockSignals(block);
    mapTypeCombo->blockSignals(block);
}

void TopControlBar::setContextBreadcrumb(const QString& domain, const QString& title) {
    if (breadcrumbDomainLabel) breadcrumbDomainLabel->setText(domain.toUpper());
    if (breadcrumbTitleLabel) breadcrumbTitleLabel->setText(title);
}

void TopControlBar::setupUi() {
    using namespace treasure::ui::theme;
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->setSpacing(12);

    setObjectName("TopControlBar");
    setStyleSheet(QString("#TopControlBar { background-color: %1; border-bottom: 1px solid %2; min-height: 44px; }").arg(SURFACE_DARK, BORDER_MUTED));

    // Dynamic Breadcrumb Widget
    auto* breadcrumbContainer = new QWidget(this);
    auto* bLayout = new QHBoxLayout(breadcrumbContainer);
    bLayout->setContentsMargins(0, 0, 0, 0);
    bLayout->setSpacing(6);

    breadcrumbDomainLabel = new QLabel("CARTOGRAPHY", breadcrumbContainer);
    breadcrumbDomainLabel->setStyleSheet(QString("color: %1; font-weight: 700; font-size: 11px; letter-spacing: 0.5px;").arg(TEXT_SECONDARY));

    auto* sepLabel = new QLabel("/", breadcrumbContainer);
    sepLabel->setStyleSheet(QString("color: %1; font-weight: 400; font-size: 12px;").arg(BORDER_MUTED));

    breadcrumbTitleLabel = new QLabel("Treasure Locator", breadcrumbContainer);
    breadcrumbTitleLabel->setStyleSheet(QString("color: %1; font-weight: 600; font-size: 13px;").arg(TEXT_PRIMARY));

    bLayout->addWidget(breadcrumbDomainLabel);
    bLayout->addWidget(sepLabel);
    bLayout->addWidget(breadcrumbTitleLabel);
    layout->addWidget(breadcrumbContainer);

    layout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    // Dropdown Pill Styling
    QString comboStyle = QString(R"(
        QComboBox {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 6px;
            color: %3;
            padding: 3px 8px;
            font-size: 12px;
            font-weight: 500;
            min-height: 24px;
        }
        QComboBox:hover {
            border-color: %4;
            background-color: %5;
        }
        QComboBox::drop-down {
            border: none;
            width: 14px;
        }
        QComboBox QAbstractItemView {
            background-color: %1;
            color: %3;
            selection-background-color: %6;
            selection-color: #ffffff;
            border: 1px solid %2;
        }
    )").arg(SURFACE_CARD, BORDER_MUTED, TEXT_PRIMARY, ACCENT_EMERALD, SURFACE_HOVER, ACCENT_TINT);

    clusterLabel = new QLabel("Cluster:", this);
    clusterLabel->setStyleSheet(QString("color: %1; font-weight: 500; font-size: 12px;").arg(TEXT_SECONDARY));
    clusterCombo = new QComboBox(this);
    clusterCombo->setObjectName("clusterCombo");
    clusterCombo->setStyleSheet(comboStyle);

    serverLabel = new QLabel("Server:", this);
    serverLabel->setStyleSheet(QString("color: %1; font-weight: 500; font-size: 12px;").arg(TEXT_SECONDARY));
    serverCombo = new QComboBox(this);
    serverCombo->setObjectName("serverCombo");
    serverCombo->setStyleSheet(comboStyle);

    mapTypeLabel = new QLabel("Layer:", this);
    mapTypeLabel->setStyleSheet(QString("color: %1; font-weight: 500; font-size: 12px;").arg(TEXT_SECONDARY));
    mapTypeCombo = new QComboBox(this);
    mapTypeCombo->setObjectName("mapTypeCombo");
    mapTypeCombo->setStyleSheet(comboStyle);

    layout->addWidget(clusterLabel);
    layout->addWidget(clusterCombo);
    layout->addWidget(serverLabel);
    layout->addWidget(serverCombo);
    layout->addWidget(mapTypeLabel);
    layout->addWidget(mapTypeCombo);

    settingsButton = new QPushButton("⚙ Settings", this);
    settingsButton->setCursor(Qt::PointingHandCursor);
    settingsButton->setStyleSheet(QString(R"(
        QPushButton {
            color: %1;
            font-weight: 600;
            font-size: 12px;
            border: 1px solid %2;
            background-color: %3;
            padding: 4px 10px;
            border-radius: 6px;
        }
        QPushButton:hover {
            background-color: %4;
            color: #ffffff;
            border-color: %5;
        }
    )").arg(TEXT_SECONDARY, BORDER_MUTED, SURFACE_CARD, SURFACE_HOVER, ACCENT_EMERALD));
    layout->addWidget(settingsButton);

    exitButton = new QPushButton("✕", this);
    exitButton->setToolTip("Exit Application");
    exitButton->setCursor(Qt::PointingHandCursor);
    exitButton->setStyleSheet(QString(R"(
        QPushButton {
            color: %1;
            font-weight: 700;
            border: 1px solid %2;
            background-color: %3;
            padding: 4px 8px;
            border-radius: 6px;
            font-size: 11px;
        }
        QPushButton:hover {
            background-color: %1;
            color: #ffffff;
            border-color: %1;
        }
    )").arg(STATUS_DANGER, BORDER_MUTED, SURFACE_CARD));
    layout->addWidget(exitButton);
}


