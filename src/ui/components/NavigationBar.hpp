#pragma once

#include <QWidget>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QPushButton>

class NavigationBar : public QWidget {
    Q_OBJECT

public:
    explicit NavigationBar(QWidget *parent = nullptr);
    ~NavigationBar() override = default;

    void addTab(const QString& name, int id);
    void selectTab(int id);

signals:
    void tabSelected(int id);

private:
    void setupUi();

    QVBoxLayout* btnLayout;
    QButtonGroup* btnGroup;
};
