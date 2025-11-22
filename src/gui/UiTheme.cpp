#include "UiTheme.h"

namespace ui {

template <typename T>
const QString& cachedStyle(T generator) {
    static const QString value = generator();
    return value;
}

const QString& dialogBackground() {
    return cachedStyle([]() {
        return QStringLiteral(
            "QDialog { "
            "    background-color: #f5f7fb; "
            "    font-family: 'PingFang SC', 'Microsoft YaHei', 'Segoe UI', sans-serif; "
            "}"
        );
    });
}

const QString& headerFrameStyle() {
    return cachedStyle([]() {
        return QStringLiteral(
            "QFrame { "
            "    background-color: #1f2a37; "
            "    border-top-left-radius: 12px; "
            "    border-top-right-radius: 12px; "
            "}"
        );
    });
}

const QString& cardFrameStyle() {
    return cachedStyle([]() {
        return QStringLiteral(
            "QFrame { "
            "    background-color: white; "
            "    border: 1px solid #e2e8f0; "
            "    border-radius: 12px; "
            "    padding: 20px; "
            "}"
        );
    });
}

const QString& softCardFrameStyle() {
    return cachedStyle([]() {
        return QStringLiteral(
            "QFrame { "
            "    background-color: #f8fafc; "
            "    border: 1px dashed #cbd5f5; "
            "    border-radius: 12px; "
            "    padding: 16px; "
            "}"
        );
    });
}

const QString& sectionTitleStyle() {
    return cachedStyle([]() {
        return QStringLiteral("color: #1f2937; font-size: 15px; font-weight: 600;");
    });
}

const QString& subtleTextStyle() {
    return cachedStyle([]() {
        return QStringLiteral("color: #94a3b8; font-size: 12px;");
    });
}

const QString& labelStyle() {
    return cachedStyle([]() {
        return QStringLiteral("color: #475569; font-weight: 600; font-size: 13px;");
    });
}

const QString& inputFieldStyle() {
    return cachedStyle([]() {
        return QStringLiteral(
            "QLineEdit, QSpinBox, QComboBox { "
            "    padding: 10px 14px; "
            "    font-size: 13px; "
            "    border: 1px solid #cbd5f5; "
            "    border-radius: 8px; "
            "    background-color: white; "
            "    color: #1f2937; "
            "}"
            "QLineEdit:focus, QSpinBox:focus, QComboBox:focus { "
            "    border-color: #3b82f6; "
            "    border-width: 2px; "
            "}"
            "QLineEdit:hover, QSpinBox:hover, QComboBox:hover { "
            "    border-color: #94a3b8; "
            "}"
        );
    });
}

const QString& primaryButtonStyle() {
    return cachedStyle([]() {
        return QStringLiteral(
            "QPushButton { "
            "    padding: 10px 26px; "
            "    border-radius: 8px; "
            "    background-color: #2563eb; "
            "    color: white; "
            "    font-weight: 600; "
            "    border: none; "
            "}"
            "QPushButton:hover { background-color: #1d4ed8; }"
            "QPushButton:pressed { background-color: #153eaf; }"
        );
    });
}

const QString& secondaryButtonStyle() {
    return cachedStyle([]() {
        return QStringLiteral(
            "QPushButton { "
            "    padding: 10px 24px; "
            "    border-radius: 8px; "
            "    background-color: #ffffff; "
            "    color: #1f2937; "
            "    font-weight: 500; "
            "    border: 1px solid #cbd5f5; "
            "}"
            "QPushButton:hover { background-color: #f8fafc; }"
        );
    });
}

const QString& pillStyle() {
    return cachedStyle([]() {
        return QStringLiteral(
            "QLabel { "
            "    padding: 4px 12px; "
            "    border-radius: 999px; "
            "    background-color: #e0f2fe; "
            "    color: #0369a1; "
            "    font-size: 11px; "
            "    font-weight: 600; "
            "}"
        );
    });
}

const QString& tableStyle() {
    return cachedStyle([]() {
        return QStringLiteral(
            "QTableView { "
            "    border: 1px solid #e2e8f0; "
            "    background-color: white; "
            "    border-radius: 10px; "
            "    gridline-color: #f1f5f9; "
            "}"
            "QTableView::item { "
            "    padding: 10px; "
            "    color: #1e293b; "
            "    font-size: 12px; "
            "}"
            "QTableView::item:hover { "
            "    background-color: #f8fafc; "
            "}"
            "QTableView::item:selected { "
            "    background-color: #2563eb; "
            "    color: white; "
            "}"
            "QTableView::item:selected:hover { "
            "    background-color: #1d4ed8; "
            "}"
            "QHeaderView::section { "
            "    background-color: #1f2a37; "
            "    color: white; "
            "    padding: 12px; "
            "    font-weight: 600; "
            "    border: none; "
            "}"
            "QHeaderView::section:horizontal { "
            "    border-left: 1px solid #0f172a; "
            "}"
            "QHeaderView::section:vertical { "
            "    border-top: 1px solid #0f172a; "
            "}"
            "QTableCornerButton::section { "
            "    background-color: #1f2a37; "
            "    border: none; "
            "}"
        );
    });
}

} // namespace ui
