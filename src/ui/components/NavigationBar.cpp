#include "NavigationBar.hpp"

NavigationBar::NavigationBar(QWidget *parent) : QWidget(parent) {
    setupUi();
}

void NavigationBar::setupUi() {
    setFixedWidth(200);
    setObjectName("NavigationBar");
    setStyleSheet("#NavigationBar { background-color: #181825; border-right: 1px solid #313244; }");

    btnLayout = new QVBoxLayout(this);
    btnLayout->setContentsMargins(10, 20, 10, 20);
    btnLayout->setSpacing(8);
    btnLayout->setAlignment(Qt::AlignTop);

    btnGroup = new QButtonGroup(this);
    btnGroup->setExclusive(true);

    connect(btnGroup, &QButtonGroup::idClicked, this, &NavigationBar::tabSelected);
}

void NavigationBar::addTab(const QString& name, int id) {
    auto* btn = new QPushButton(name, this);
    btn->setCheckable(true);
    btn->setMinimumHeight(40);
    
    // Custom styling for nav buttons
    btn->setStyleSheet(R"(
        QPushButton {
            text-align: left;
            padding-left: 16px;
            background-color: transparent;
            border: none;
            border-radius: 8px;
            color: #a6adc8;
            font-size: 14px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #313244;
            color: #cdd6f4;
        }
        QPushButton:checked {
            background-color: #8aadf4;
            color: #11111b;
            font-weight: bold;
        }
    )");

    btnGroup->addButton(btn, id);
    btnLayout->addWidget(btn);
}

void NavigationBar::selectTab(int id) {
    if (auto* btn = btnGroup->button(id)) {
        btn->setChecked(true);
        emit tabSelected(id);
    }
}
