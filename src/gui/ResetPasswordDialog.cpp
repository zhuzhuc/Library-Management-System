#include "ResetPasswordDialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include "UiTheme.h"

ResetPasswordDialog::ResetPasswordDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("重置密码");
    setMinimumSize(400, 350);
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
    QLabel* titleLabel = new QLabel("重置用户密码", this);
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
    
    QFrame* formFrame = new QFrame(this);
    formFrame->setStyleSheet("QFrame { background-color: transparent; }");
    QFormLayout* formLayout = new QFormLayout(formFrame);
    formLayout->setSpacing(18);
    formLayout->setLabelAlignment(Qt::AlignRight);
    
    QString inputStyle = ui::inputFieldStyle();
    QString labelStyle = ui::labelStyle();
    
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("输入要重置密码的用户名");
    usernameEdit->setStyleSheet(inputStyle);
    
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("输入新密码（至少6位）");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(inputStyle);
    
    QLabel* usernameLabel = new QLabel("用户名:", this);
    usernameLabel->setStyleSheet(labelStyle);
    QLabel* passwordLabel = new QLabel("新密码:", this);
    passwordLabel->setStyleSheet(labelStyle);
    
    formLayout->addRow(usernameLabel, usernameEdit);
    formLayout->addRow(passwordLabel, passwordEdit);
    
    contentLayout->addWidget(formFrame);
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
    
    QPushButton* resetBtn = new QPushButton("重置密码", this);
    resetBtn->setCursor(Qt::PointingHandCursor);
    resetBtn->setStyleSheet(ui::primaryButtonStyle());
    
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(resetBtn);
    mainLayout->addWidget(buttonFrame);
    
    QDialogButtonBox* buttons = new QDialogButtonBox(this);
    buttons->addButton(resetBtn, QDialogButtonBox::AcceptRole);
    buttons->addButton(cancelBtn, QDialogButtonBox::RejectRole);
    
    connect(resetBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    
    usernameEdit->setFocus();
}

QString ResetPasswordDialog::getUsername() const {
    return usernameEdit->text();
}

QString ResetPasswordDialog::getNewPassword() const {
    return passwordEdit->text();
}
