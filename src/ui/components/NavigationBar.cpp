#include "NavigationBar.hpp"
#include "../ThemeTokens.hpp"
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>

NavigationBar::NavigationBar(QWidget *parent) : QWidget(parent) {
    setupUi();
}

void NavigationBar::setupUi() {
    using namespace treasure::ui::theme;
    setFixedWidth(220);
    setObjectName("NavigationBar");
    setStyleSheet(QString("#NavigationBar { background-color: %1; border-right: 1px solid %2; }").arg(BG_SIDEBAR, BORDER_MUTED));

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 16, 12, 12);
    mainLayout->setSpacing(0);

    // 1. Brand Header
    auto* brandWidget = new QWidget(this);
    auto* brandLayout = new QHBoxLayout(brandWidget);
    brandLayout->setContentsMargins(4, 0, 4, 16);
    brandLayout->setSpacing(10);

    auto* logoBadge = new QLabel("WE", brandWidget);
    logoBadge->setFixedSize(32, 32);
    logoBadge->setAlignment(Qt::AlignCenter);
    logoBadge->setStyleSheet(QString(R"(
        background-color: %1;
        color: %2;
        font-weight: 900;
        font-size: 13px;
        border-radius: 6px;
        border: 1px solid %3;
    )").arg(ACCENT_TINT, ACCENT_MINT, ACCENT_EMERALD));

    auto* titleCol = new QVBoxLayout();
    titleCol->setSpacing(1);
    titleCol->setContentsMargins(0, 0, 0, 0);

    auto* brandTitle = new QLabel("WurmExplorer", brandWidget);
    brandTitle->setStyleSheet(QString("color: %1; font-weight: 700; font-size: 14px;").arg(TEXT_PRIMARY));

    auto* brandSub = new QLabel("PRO WORKSTATION", brandWidget);
    brandSub->setStyleSheet(QString("color: %1; font-weight: 700; font-size: 9px; letter-spacing: 1px;").arg(TEXT_SECONDARY));

    titleCol->addWidget(brandTitle);
    titleCol->addWidget(brandSub);

    brandLayout->addWidget(logoBadge);
    brandLayout->addLayout(titleCol);
    brandLayout->addStretch();
    mainLayout->addWidget(brandWidget);

    // Divider
    auto* topDivider = new QFrame(this);
    topDivider->setFrameShape(QFrame::HLine);
    topDivider->setStyleSheet(QString("background-color: %1; max-height: 1px; margin-bottom: 8px;").arg(BORDER_MUTED));
    mainLayout->addWidget(topDivider);

    // 2. Navigation Tab Button Area
    btnLayout = new QVBoxLayout();
    btnLayout->setContentsMargins(0, 4, 0, 4);
    btnLayout->setSpacing(3);
    btnLayout->setAlignment(Qt::AlignTop);

    btnGroup = new QButtonGroup(this);
    btnGroup->setExclusive(true);
    connect(btnGroup, &QButtonGroup::idClicked, this, &NavigationBar::tabSelected);

    mainLayout->addLayout(btnLayout, 1); // Expand to fill available vertical space

    // 3. Player Profile & Status Footer Card
    auto* bottomDivider = new QFrame(this);
    bottomDivider->setFrameShape(QFrame::HLine);
    bottomDivider->setStyleSheet(QString("background-color: %1; max-height: 1px; margin-top: 8px; margin-bottom: 8px;").arg(BORDER_MUTED));
    mainLayout->addWidget(bottomDivider);

    auto* profileCard = new QWidget(this);
    profileCard->setStyleSheet(QString(R"(
        QWidget {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 8px;
        }
    )").arg(SURFACE_DARK, BORDER_MUTED));
    auto* profileLayout = new QHBoxLayout(profileCard);
    profileLayout->setContentsMargins(8, 8, 8, 8);
    profileLayout->setSpacing(8);

    playerAvatarLabel = new QLabel("PB", profileCard);
    playerAvatarLabel->setFixedSize(30, 30);
    playerAvatarLabel->setAlignment(Qt::AlignCenter);
    playerAvatarLabel->setStyleSheet(QString(R"(
        background-color: %1;
        color: %2;
        font-weight: bold;
        font-size: 11px;
        border-radius: 15px;
        border: 1px solid %3;
    )").arg(ACCENT_TINT, ACCENT_MINT, ACCENT_EMERALD));

    auto* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(1);
    infoLayout->setContentsMargins(0, 0, 0, 0);

    playerNameLabel = new QLabel("polarbear", profileCard);
    playerNameLabel->setStyleSheet(QString("color: %1; font-weight: 600; font-size: 12px; border: none;").arg(TEXT_PRIMARY));

    playerStatusLabel = new QLabel("● Online", profileCard);
    playerStatusLabel->setStyleSheet(QString("color: %1; font-size: 10px; border: none;").arg(STATUS_SUCCESS));

    infoLayout->addWidget(playerNameLabel);
    infoLayout->addWidget(playerStatusLabel);

    auto* gearBtn = new QPushButton("⚙", profileCard);
    gearBtn->setFixedSize(26, 26);
    gearBtn->setCursor(Qt::PointingHandCursor);
    gearBtn->setToolTip("Open Settings");
    gearBtn->setStyleSheet(QString(R"(
        QPushButton {
            color: %1;
            background-color: %2;
            border: 1px solid %3;
            border-radius: 4px;
            font-size: 13px;
        }
        QPushButton:hover {
            color: #ffffff;
            background-color: %4;
            border-color: %5;
        }
    )").arg(TEXT_SECONDARY, SURFACE_CARD, BORDER_MUTED, SURFACE_HOVER, ACCENT_EMERALD));
    connect(gearBtn, &QPushButton::clicked, this, &NavigationBar::settingsRequested);

    profileLayout->addWidget(playerAvatarLabel);
    profileLayout->addLayout(infoLayout, 1);
    profileLayout->addWidget(gearBtn);

    mainLayout->addWidget(profileCard);
}

void NavigationBar::addSectionHeader(const QString& title) {
    using namespace treasure::ui::theme;
    auto* header = new QLabel(title.toUpper(), this);
    header->setStyleSheet(QString(R"(
        color: %1;
        font-size: 10px;
        font-weight: 700;
        letter-spacing: 1px;
        padding-left: 8px;
        margin-top: 10px;
        margin-bottom: 2px;
        background: transparent;
        border: none;
    )").arg(TEXT_SECONDARY));
    btnLayout->addWidget(header);
}

void NavigationBar::addTab(const QString& name, int id) {
    using namespace treasure::ui::theme;
    auto* btn = new QPushButton(name, this);
    btn->setCheckable(true);
    btn->setMinimumHeight(34);
    btn->setCursor(Qt::PointingHandCursor);
    
    btn->setStyleSheet(QString(R"(
        QPushButton {
            text-align: left;
            padding-left: 12px;
            background-color: transparent;
            border: 1px solid transparent;
            border-radius: 6px;
            color: %1;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: %2;
            color: %3;
        }
        QPushButton:checked {
            background-color: %4;
            border: 1px solid %5;
            border-left: 3px solid %5;
            color: %6;
            font-weight: 600;
            padding-left: 10px;
        }
    )").arg(TEXT_SECONDARY, SURFACE_CARD, TEXT_PRIMARY, ACCENT_TINT, ACCENT_EMERALD, ACCENT_MINT));

    btnGroup->addButton(btn, id);
    btnLayout->addWidget(btn);
}

void NavigationBar::selectTab(int id) {
    if (auto* btn = btnGroup->button(id)) {
        btn->setChecked(true);
        emit tabSelected(id);
    }
}

int NavigationBar::currentTab() const {
    return btnGroup ? btnGroup->checkedId() : -1;
}

void NavigationBar::setPlayerInfo(const QString& playerName, const QString& serverStatus) {
    if (playerNameLabel) {
        playerNameLabel->setText(playerName);
        if (playerAvatarLabel) {
            QString initials = playerName.left(2).toUpper();
            playerAvatarLabel->setText(initials.isEmpty() ? "WE" : initials);
        }
    }
    if (playerStatusLabel) {
        playerStatusLabel->setText(serverStatus);
    }
}

