#pragma once

#include <QWidget>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

class NavigationBar : public QWidget {
    Q_OBJECT

public:
    explicit NavigationBar(QWidget *parent = nullptr);
    ~NavigationBar() override = default;

    void addSectionHeader(const QString& title);
    void addTab(const QString& name, int id);
    void selectTab(int id);
    int currentTab() const;
    void setPlayerInfo(const QString& playerName, const QString& serverStatus);

signals:
    void tabSelected(int id);
    void settingsRequested();

private:
    void setupUi();

    QVBoxLayout* mainLayout;
    QVBoxLayout* btnLayout;
    QButtonGroup* btnGroup;
    QLabel* playerAvatarLabel = nullptr;
    QLabel* playerNameLabel = nullptr;
    QLabel* playerStatusLabel = nullptr;
};
