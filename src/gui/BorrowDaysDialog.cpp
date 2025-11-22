#include "BorrowDaysDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QFrame>
#include <QDialogButtonBox>
#include <QPushButton>
#include "UiTheme.h"

BorrowDaysDialog::BorrowDaysDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("选择借阅天数");
    setMinimumSize(400, 280);
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    setStyleSheet(ui::dialogBackground());
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // 标题区域
    QFrame* titleFrame = new QFrame(this);
    titleFrame->setFixedHeight(60);
    titleFrame->setStyleSheet(ui::headerFrameStyle());
    QHBoxLayout* titleLayout = new QHBoxLayout(titleFrame);
    titleLayout->setContentsMargins(20, 15, 20, 15);
    QLabel* titleLabel = new QLabel("选择借阅天数", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: white; background: transparent;");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    mainLayout->addWidget(titleFrame);
    
    // 内容区域
    QFrame* contentFrame = new QFrame(this);
    contentFrame->setStyleSheet(ui::cardFrameStyle());
    QVBoxLayout* contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(30, 25, 30, 25);
    
    QLabel* label = new QLabel("借阅天数 (1-7天):", this);
    label->setStyleSheet(ui::labelStyle());
    contentLayout->addWidget(label);
    
    daysSpin = new QSpinBox(this);
    daysSpin->setMinimum(1);
    daysSpin->setMaximum(7);
    daysSpin->setValue(7);
    daysSpin->setSuffix(" 天");
    daysSpin->setStyleSheet(ui::inputFieldStyle());
    contentLayout->addWidget(daysSpin);
    
    QLabel* hintLabel = new QLabel("提示: 每本书最多可借7天。", this);
    hintLabel->setStyleSheet(ui::subtleTextStyle());
    contentLayout->addWidget(hintLabel);
    
    contentLayout->addStretch();
    mainLayout->addWidget(contentFrame, 1);
    
    // 按钮区域
    QFrame* buttonFrame = new QFrame(this);
    buttonFrame->setStyleSheet("QFrame { background-color: transparent; }");
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonFrame);
    buttonLayout->setContentsMargins(30, 15, 30, 20);
    buttonLayout->addStretch();
    
    QPushButton* cancelBtn = new QPushButton("取消", this);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(ui::secondaryButtonStyle());
    
    QPushButton* okBtn = new QPushButton("确定", this);
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setStyleSheet(ui::primaryButtonStyle());
    
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(okBtn);
    mainLayout->addWidget(buttonFrame);
    
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    
    daysSpin->setFocus();
}

int BorrowDaysDialog::getDays() const {
    return daysSpin->value();
}
