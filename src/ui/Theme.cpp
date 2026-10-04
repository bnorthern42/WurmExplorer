#include "Theme.hpp"
#include "ThemeTokens.hpp"
#include <QApplication>
#include <QPalette>
#include <QFont>
#include <QString>

void Theme::apply(QApplication& app) {
    QFont font("Segoe UI");
    font.setPointSize(10);
    app.setFont(font);

    using namespace treasure::ui::theme;

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(BG_DARK));
    palette.setColor(QPalette::WindowText, QColor(TEXT_PRIMARY));
    palette.setColor(QPalette::Base, QColor(SURFACE_CARD));
    palette.setColor(QPalette::AlternateBase, QColor(SURFACE_DARK));
    palette.setColor(QPalette::ToolTipBase, QColor(SURFACE_CARD));
    palette.setColor(QPalette::ToolTipText, QColor(TEXT_PRIMARY));
    palette.setColor(QPalette::Text, QColor(TEXT_PRIMARY));
    palette.setColor(QPalette::Button, QColor(SURFACE_CARD));
    palette.setColor(QPalette::ButtonText, QColor(TEXT_PRIMARY));
    palette.setColor(QPalette::BrightText, QColor(ACCENT_MINT));
    palette.setColor(QPalette::Highlight, QColor(ACCENT_EMERALD));
    palette.setColor(QPalette::HighlightedText, QColor(TEXT_ON_ACCENT));
    app.setPalette(palette);

    QString styleSheet = QString(R"(
        QWidget {
            background-color: %1;
            color: %2;
        }
        QMainWindow, QDialog {
            background-color: %1;
        }
        QGroupBox {
            border: 1px solid %3;
            border-radius: 8px;
            margin-top: 14px;
            padding-top: 14px;
            font-weight: 600;
            background-color: %4;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
            color: %5;
        }
        QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox {
            background-color: %6;
            border: 1px solid %3;
            border-bottom: 2px solid %3;
            border-radius: 6px;
            padding: 6px 10px;
            color: %2;
            selection-background-color: %7;
            selection-color: %8;
        }
        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus {
            border-color: %7;
            border-bottom: 2px solid %5;
        }
        QComboBox {
            background-color: %6;
            border: 1px solid %3;
            border-radius: 6px;
            padding: 6px 10px;
            color: %2;
        }
        QComboBox:hover {
            border-color: %7;
        }
        QComboBox::drop-down {
            border: 0px;
            width: 24px;
        }
        QComboBox QAbstractItemView {
            background-color: %4;
            border: 1px solid %3;
            selection-background-color: %9;
            selection-color: %5;
            color: %2;
            padding: 4px;
        }
        QPushButton, QToolButton {
            background-color: %6;
            border: 1px solid %3;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: 500;
            color: %2;
        }
        QPushButton:hover, QToolButton:hover {
            background-color: %10;
            border-color: %7;
            color: #ffffff;
        }
        QPushButton:pressed, QToolButton:pressed {
            background-color: %11;
            border-color: %7;
            color: #ffffff;
        }
        QPushButton:checked, QToolButton:checked {
            background-color: %9;
            border: 1px solid %7;
            color: %5;
            font-weight: 600;
        }
        QPushButton:disabled, QToolButton:disabled {
            background-color: %1;
            border-color: %3;
            color: %12;
        }
        QTabWidget::pane {
            border: 1px solid %3;
            border-radius: 8px;
            background-color: %1;
            top: -1px;
        }
        QTabBar::tab {
            background-color: %4;
            border: 1px solid %3;
            padding: 8px 16px;
            margin-right: 4px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            color: %12;
        }
        QTabBar::tab:hover {
            background-color: %6;
            color: %2;
            border-bottom: 2px solid %7;
        }
        QTabBar::tab:selected {
            background-color: %6;
            border-color: %3;
            border-bottom: 3px solid %7;
            color: %5;
            font-weight: 600;
        }
        QListWidget, QTableView, QTreeView {
            background-color: %4;
            border: 1px solid %3;
            border-radius: 6px;
            color: %2;
            gridline-color: %3;
        }
        QListWidget::item:selected, QTableView::item:selected, QTreeView::item:selected {
            background-color: %9;
            color: %5;
            border: 1px solid %7;
        }
        QListWidget::item:hover, QTableView::item:hover, QTreeView::item:hover {
            background-color: %10;
        }
        QHeaderView::section {
            background-color: %6;
            color: %12;
            padding: 6px 8px;
            border: 0px;
            border-right: 1px solid %3;
            border-bottom: 2px solid %3;
            font-weight: 600;
        }
        QCheckBox, QRadioButton {
            spacing: 8px;
            color: %2;
        }
        QScrollBar:vertical, QScrollBar:horizontal {
            background: %1;
            border: 0px;
            margin: 0px;
        }
        QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
            background: %3;
            border-radius: 4px;
            min-height: 24px;
            min-width: 24px;
        }
        QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover {
            background: %7;
        }
        QProgressBar {
            background-color: %6;
            border: 1px solid %3;
            border-radius: 6px;
            text-align: center;
            color: %2;
            font-weight: 600;
        }
        QProgressBar::chunk {
            background-color: %7;
            border-radius: 5px;
        }
        QLabel[muted="true"] {
            color: %12;
        }
        QStatusBar {
            background-color: %1;
            border-top: 1px solid %3;
            color: %12;
        }
        QMenu {
            background-color: %4;
            border: 1px solid %3;
            color: %2;
            padding: 4px;
        }
        QMenu::item:selected {
            background-color: %9;
            color: %5;
        }
    )").arg(
        BG_DARK,         // %1
        TEXT_PRIMARY,    // %2
        BORDER_MUTED,    // %3
        SURFACE_DARK,    // %4
        ACCENT_MINT,     // %5
        SURFACE_CARD,    // %6
        ACCENT_EMERALD,  // %7
        TEXT_ON_ACCENT,  // %8
        ACCENT_TINT,     // %9
        SURFACE_HOVER,   // %10
        ACCENT_PRESSED,  // %11
        TEXT_SECONDARY   // %12
    );

    app.setStyleSheet(styleSheet);
}
