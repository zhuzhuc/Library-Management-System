#include "AppearanceDialog.h"

#include "AppSettings.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

AppearanceDialog::AppearanceDialog(QWidget* parent)
    : QDialog(parent),
      loginPathEdit(new QLineEdit(this)),
      mainPathEdit(new QLineEdit(this)) {
    setWindowTitle("界面设置");
    setModal(true);
    setMinimumWidth(500);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);

    auto createSection = [this](const QString& title,
                                QLineEdit* pathEdit,
                                auto chooseSlot,
                                auto resetSlot) {
        QFrame* frame = new QFrame(this);
        frame->setStyleSheet(
            "QFrame { "
            "    border: 1px solid #e2e8f0; "
            "    border-radius: 8px; "
            "    background-color: white; "
            "}"
        );
        QVBoxLayout* frameLayout = new QVBoxLayout(frame);
        frameLayout->setSpacing(8);
        frameLayout->setContentsMargins(16, 16, 16, 16);

        QLabel* titleLabel = new QLabel(title, frame);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(12);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        frameLayout->addWidget(titleLabel);

        pathEdit->setReadOnly(true);
        pathEdit->setPlaceholderText("尚未设置自定义背景");
        pathEdit->setStyleSheet(
            "QLineEdit { "
            "    background-color: #f8fafc; "
            "    border-radius: 4px; "
            "    border: 1px solid #e2e8f0; "
            "    padding: 6px; "
            "}"
        );
        frameLayout->addWidget(pathEdit);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing(10);
        QPushButton* chooseBtn = new QPushButton("选择图片", frame);
        QPushButton* resetBtn = new QPushButton("恢复默认", frame);
        chooseBtn->setCursor(Qt::PointingHandCursor);
        resetBtn->setCursor(Qt::PointingHandCursor);

        chooseBtn->setStyleSheet(
            "QPushButton { "
            "    padding: 6px 16px; "
            "    border-radius: 4px; "
            "    background-color: #3498db; "
            "    color: white; "
            "    border: none; "
            "}"
            "QPushButton:hover { background-color: #2980b9; }"
        );
        resetBtn->setStyleSheet(
            "QPushButton { "
            "    padding: 6px 16px; "
            "    border-radius: 4px; "
            "    background-color: #ecf0f1; "
            "    color: #2c3e50; "
            "    border: none; "
            "}"
            "QPushButton:hover { background-color: #dfe6e9; }"
        );

        connect(chooseBtn, &QPushButton::clicked, this, chooseSlot);
        connect(resetBtn, &QPushButton::clicked, this, resetSlot);

        buttonLayout->addWidget(chooseBtn);
        buttonLayout->addWidget(resetBtn);
        buttonLayout->addStretch();
        frameLayout->addLayout(buttonLayout);

        return frame;
    };

    mainLayout->addWidget(createSection("登录界面背景", loginPathEdit,
                                        [this]() { chooseLoginBackground(); },
                                        [this]() { resetLoginBackground(); }));
    mainLayout->addWidget(createSection("主界面背景", mainPathEdit,
                                        [this]() { chooseMainBackground(); },
                                        [this]() { resetMainBackground(); }));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    mainLayout->addWidget(buttonBox);

    updatePreviewFields();
}

void AppearanceDialog::chooseLoginBackground() {
    QString filter = "Images (*.png *.jpg *.jpeg *.bmp *.gif)";
    QString filePath = QFileDialog::getOpenFileName(this, "选择登录背景", QString(), filter);
    if (filePath.isEmpty()) {
        return;
    }
    AppSettings::instance().setLoginBackgroundPath(filePath);
    updatePreviewFields();
    QMessageBox::information(this, "设置成功", "登录界面背景已更新，下次打开登录窗体时生效。");
}

void AppearanceDialog::chooseMainBackground() {
    QString filter = "Images (*.png *.jpg *.jpeg *.bmp *.gif)";
    QString filePath = QFileDialog::getOpenFileName(this, "选择主界面背景", QString(), filter);
    if (filePath.isEmpty()) {
        return;
    }
    AppSettings::instance().setMainBackgroundPath(filePath);
    updatePreviewFields();
}

void AppearanceDialog::resetLoginBackground() {
    AppSettings::instance().resetLoginBackground();
    updatePreviewFields();
}

void AppearanceDialog::resetMainBackground() {
    AppSettings::instance().resetMainBackground();
    updatePreviewFields();
}

void AppearanceDialog::updatePreviewFields() {
    loginPathEdit->setText(AppSettings::instance().loginBackgroundPath());
    mainPathEdit->setText(AppSettings::instance().mainBackgroundPath());
}
