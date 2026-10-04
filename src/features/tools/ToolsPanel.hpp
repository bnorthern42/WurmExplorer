#pragma once

#include <QWidget>
#include <QStackedWidget>
#include <QButtonGroup>

namespace tools {

class GrinderWidget;

class ToolsPanel : public QWidget {
    Q_OBJECT
public:
    explicit ToolsPanel(QWidget* parent = nullptr);
    GrinderWidget* getGrinderWidget() const { return m_grinderWidget; }

private slots:
    void onTabChanged(int id);

private:
    void setupUi();

    QButtonGroup* m_tabGroup = nullptr;
    QStackedWidget* m_stack = nullptr;
    GrinderWidget* m_grinderWidget = nullptr;
};

} // namespace tools
