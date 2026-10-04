#include "ToolsPanel.hpp"
#include "ImpCalculatorWidget.hpp"
#include "BridgePillarWidget.hpp"
#include "GrinderWidget.hpp"
#include "../../ui/ThemeTokens.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

namespace tools {

using namespace treasure::ui;

ToolsPanel::ToolsPanel(QWidget* parent)
    : QWidget(parent) {
    setupUi();
}

void ToolsPanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(14);

    // Segmented Navigation Header
    auto* navContainer = new QWidget(this);
    navContainer->setStyleSheet(QString(
        "background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 4px;"
    ).arg(theme::SURFACE_DARK, theme::BORDER_MUTED));
    auto* navLayout = new QHBoxLayout(navContainer);
    navLayout->setContentsMargins(4, 4, 4, 4);
    navLayout->setSpacing(6);

    m_tabGroup = new QButtonGroup(this);
    m_tabGroup->setExclusive(true);

    const QString btnStyle = QString(R"(
        QPushButton {
            background-color: transparent;
            color: %1;
            font-size: 13px;
            font-weight: 600;
            border-radius: 6px;
            padding: 8px 16px;
            border: none;
        }
        QPushButton:hover {
            background-color: %2;
            color: %3;
        }
        QPushButton:checked {
            background-color: %4;
            color: %5;
            font-weight: bold;
        }
    )").arg(theme::TEXT_SECONDARY, theme::SURFACE_HOVER, theme::TEXT_PRIMARY, theme::ACCENT_TINT, theme::ACCENT_MINT);

    auto* btnImp = new QPushButton("🔨 Imp Calculator", navContainer);
    btnImp->setCheckable(true);
    btnImp->setChecked(true);
    btnImp->setStyleSheet(btnStyle);
    m_tabGroup->addButton(btnImp, 0);
    navLayout->addWidget(btnImp);

    auto* btnBridge = new QPushButton("🌉 Bridge Dirt Pillar", navContainer);
    btnBridge->setCheckable(true);
    btnBridge->setStyleSheet(btnStyle);
    m_tabGroup->addButton(btnBridge, 1);
    navLayout->addWidget(btnBridge);

    auto* btnGrinder = new QPushButton("⚙ Mechanics Grinder", navContainer);
    btnGrinder->setCheckable(true);
    btnGrinder->setStyleSheet(btnStyle);
    m_tabGroup->addButton(btnGrinder, 2);
    navLayout->addWidget(btnGrinder);

    navLayout->addStretch(1);
    mainLayout->addWidget(navContainer);

    // Stacked Pages
    m_stack = new QStackedWidget(this);
    m_stack->addWidget(new ImpCalculatorWidget(this));
    m_stack->addWidget(new BridgePillarWidget(this));
    m_grinderWidget = new GrinderWidget(this);
    m_stack->addWidget(m_grinderWidget);

    mainLayout->addWidget(m_stack, 1);

    connect(m_tabGroup, &QButtonGroup::idClicked, this, &ToolsPanel::onTabChanged);
}

void ToolsPanel::onTabChanged(int id) {
    if (id >= 0 && id < m_stack->count()) {
        m_stack->setCurrentIndex(id);
    }
}

void ToolsPanel::selectToolTab(int id) {
    if (m_tabGroup) {
        if (auto* btn = m_tabGroup->button(id)) {
            btn->setChecked(true);
            onTabChanged(id);
        }
    }
}

} // namespace tools
