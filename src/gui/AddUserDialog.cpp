#include "AddUserDialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include "UiTheme.h"

AddUserDialog::AddUserDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("添加用户");
    setMinimumSize(450, 600);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    setStyleSheet(ui::dialogBackground());
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // 标题区域
    QFrame* titleFrame = new QFrame(this);
    titleFrame->setFixedHeight(64);
    titleFrame->setStyleSheet(ui::headerFrameStyle());
    QHBoxLayout* titleLayout = new QHBoxLayout(titleFrame);
    titleLayout->setContentsMargins(20, 15, 20, 15);
    QLabel* titleLabel = new QLabel("添加新用户", this);
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
    QFormLayout* layout = new QFormLayout(formFrame);
    layout->setSpacing(18);
    layout->setLabelAlignment(Qt::AlignRight);
    
    QString inputStyle = ui::inputFieldStyle();
    QString labelStyle = ui::labelStyle();
    
    typeBox = new QComboBox(this);
    typeBox->addItem("学生 (Student)", "student");
    typeBox->addItem("教师 (Teacher)", "teacher");
    typeBox->setStyleSheet(inputStyle);
    
    idEdit = new QLineEdit(this);
    idEdit->setPlaceholderText("输入用户ID");
    idEdit->setStyleSheet(inputStyle);
    
    nameEdit = new QLineEdit(this);
    nameEdit->setPlaceholderText("输入姓名");
    nameEdit->setStyleSheet(inputStyle);
    
    deptEdit = new QLineEdit(this);
    deptEdit->setPlaceholderText("输入院系");
    deptEdit->setStyleSheet(inputStyle);
    
    extraEdit = new QLineEdit(this);
    extraEdit->setPlaceholderText("学生：专业 / 教师：职称");
    extraEdit->setStyleSheet(inputStyle);
    
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("设置登录密码（至少6位）");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(inputStyle);
    
    limitSpin = new QSpinBox(this);
    limitSpin->setMinimum(1);
    limitSpin->setMaximum(100);
    limitSpin->setValue(5);
    limitSpin->setStyleSheet(inputStyle);

    QLabel* typeLabel = new QLabel("类型:", this);
    typeLabel->setStyleSheet(labelStyle);
    QLabel* idLabel = new QLabel("用户ID:", this);
    idLabel->setStyleSheet(labelStyle);
    QLabel* nameLabel = new QLabel("姓名:", this);
    nameLabel->setStyleSheet(labelStyle);
    QLabel* deptLabel = new QLabel("院系:", this);
    deptLabel->setStyleSheet(labelStyle);
    QLabel* extraLabel = new QLabel("专业/职称:", this);
    extraLabel->setStyleSheet(labelStyle);
    QLabel* passwordLabel = new QLabel("登录密码:", this);
    passwordLabel->setStyleSheet(labelStyle);
    QLabel* limitLabel = new QLabel("最大借阅数:", this);
    limitLabel->setStyleSheet(labelStyle);

    layout->addRow(typeLabel, typeBox);
    layout->addRow(idLabel, idEdit);
    layout->addRow(nameLabel, nameEdit);
    layout->addRow(deptLabel, deptEdit);
    layout->addRow(extraLabel, extraEdit);
    layout->addRow(passwordLabel, passwordEdit);
    layout->addRow(limitLabel, limitSpin);
    
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
    
    QPushButton* addBtn = new QPushButton("保存", this);
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setStyleSheet(ui::primaryButtonStyle());
    
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(addBtn);
    mainLayout->addWidget(buttonFrame);
    
    QDialogButtonBox* buttons = new QDialogButtonBox(this);
    buttons->addButton(addBtn, QDialogButtonBox::AcceptRole);
    buttons->addButton(cancelBtn, QDialogButtonBox::RejectRole);
    
    connect(addBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

QString AddUserDialog::getType() const { return typeBox->currentData().toString(); }
QString AddUserDialog::getId() const { return idEdit->text(); }
QString AddUserDialog::getName() const { return nameEdit->text(); }
QString AddUserDialog::getDept() const { return deptEdit->text(); }
QString AddUserDialog::getExtra() const { return extraEdit->text(); }
int AddUserDialog::getLimit() const { return limitSpin->value(); }
QString AddUserDialog::getPassword() const { return passwordEdit->text(); }
