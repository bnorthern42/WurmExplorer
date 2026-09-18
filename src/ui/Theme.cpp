#include "Theme.hpp"
#include <QApplication>
#include <QPalette>
#include <QFont>
#include <QString>

void Theme::apply(QApplication& app) {
    QFont font("Segoe UI");
    font.setPointSize(10);
    app.setFont(font);

    // Catppuccin Macchiato inspired palette
    const QString bg_color = "#24273a";
    const QString surface_color = "#363a4f";
    const QString surface_hover = "#494d64";
    const QString text_color = "#cad3f5";
    const QString accent_color = "#8aadf4";
    const QString border_color = "#494d64";

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(bg_color));
    palette.setColor(QPalette::WindowText, QColor(text_color));
    palette.setColor(QPalette::Base, QColor(surface_color));
    palette.setColor(QPalette::AlternateBase, QColor(bg_color));
    palette.setColor(QPalette::ToolTipBase, QColor(surface_color));
    palette.setColor(QPalette::ToolTipText, QColor(text_color));
    palette.setColor(QPalette::Text, QColor(text_color));
    palette.setColor(QPalette::Button, QColor(surface_color));
    palette.setColor(QPalette::ButtonText, QColor(text_color));
    palette.setColor(QPalette::BrightText, QColor("#ffffff"));
    palette.setColor(QPalette::Highlight, QColor(accent_color));
    palette.setColor(QPalette::HighlightedText, QColor(bg_color));
    app.setPalette(palette);

    QString styleSheet = QString(R"(
        QWidget {
            background: %1;
            color: %2;
        }
        QMainWindow, QDialog {
            background: %1;
        }
        QGroupBox {
            border: 1px solid %3;
            border-radius: 8px;
            margin-top: 14px;
            padding-top: 14px;
            font-weight: 600;
            background: %1;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
            color: %4;
        }
        QLineEdit, QTextEdit, QPlainTextEdit, QListWidget, QComboBox, QSpinBox, QDoubleSpinBox {
            background: %5;
            border: 1px solid %3;
            border-radius: 6px;
            padding: 6px 10px;
            selection-background-color: %4;
            selection-color: %1;
        }
        QComboBox::drop-down {
            border: 0;
            width: 24px;
        }
        QPushButton, QToolButton {
            background: %5;
            border: 1px solid %3;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: 500;
        }
        QPushButton:hover, QToolButton:hover {
            background: %6;
        }
        QPushButton:pressed, QToolButton:pressed {
            background: %4;
            border-color: %4;
            color: %1;
        }
        QPushButton:checked, QToolButton:checked {
            background: %4;
            border-color: %4;
            color: %1;
        }
        QTabWidget::pane {
            border: 1px solid %3;
            border-radius: 8px;
            background: %1;
            top: -1px;
        }
        QTabBar::tab {
            background: %5;
            border: 1px solid %3;
            padding: 8px 16px;
            margin-right: 4px;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
        }
        QTabBar::tab:selected {
            background: %4;
            border-color: %4;
            color: %1;
            font-weight: 600;
        }
        QCheckBox, QRadioButton {
            spacing: 8px;
        }
        QScrollBar:vertical, QScrollBar:horizontal {
            background: %1;
            border: 0;
            margin: 0;
        }
        QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
            background: %3;
            border-radius: 5px;
            min-height: 20px;
            min-width: 20px;
        }
        QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover {
            background: %6;
        }
        QLabel[muted="true"] {
            color: #8a8d9e;
        }
        QStatusBar {
            background: %5;
            border-top: 1px solid %3;
            color: %2;
        }
    )").arg(bg_color, text_color, border_color, accent_color, surface_color, surface_hover);

    app.setStyleSheet(styleSheet);
}
