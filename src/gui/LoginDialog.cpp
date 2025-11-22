#include "LoginDialog.h"

#include "AppSettings.h"
#include "UiTheme.h"

#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QFile>
#include <QResizeEvent>

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("图书馆管理系统 - 用户登录");
    setMinimumSize(420, 500);
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setStyleSheet(ui::dialogBackground());
    applyBackground();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ===== 标题区域 =====
    QFrame* titleFrame = new QFrame(this);
    titleFrame->setFixedHeight(100);
    titleFrame->setStyleSheet(
        "QFrame {"
        "   background-color: #2c3e50;"
        "}"
    );

    QVBoxLayout* titleLayout = new QVBoxLayout(titleFrame);
    titleLayout->setSpacing(5);
    titleLayout->setContentsMargins(20, 15, 20, 15);
    
    QLabel* titleLabel = new QLabel("图书馆管理系统", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: white; margin: 0;");
    titleLayout->addWidget(titleLabel);

    QLabel* subtitleLabel = new QLabel("Library Management System, Made by zhuzhuc", this);
    QFont subtitleFont = subtitleLabel->font();
    subtitleFont.setPointSize(10);
    subtitleFont.setItalic(true);
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("color: #bdc3c7; margin: 0;");
    titleLayout->addWidget(subtitleLabel);

    mainLayout->addWidget(titleFrame);

    // ===== 表单区域 =====
    QFrame* formFrame = new QFrame(this);
    formFrame->setStyleSheet(
        "QFrame {"
        "   background-color: white;"
        "}"
    );

    QVBoxLayout* formMainLayout = new QVBoxLayout(formFrame);
    formMainLayout->setSpacing(15);
    formMainLayout->setContentsMargins(30, 25, 30, 25);

    // 用户类型
    QLabel* userTypeLabel = new QLabel("用户类型:", this);
    userTypeLabel->setStyleSheet("color: #2c3e50; font-weight: 600; font-size: 13px;");
    formMainLayout->addWidget(userTypeLabel);

    QHBoxLayout* userTypeLayout = new QHBoxLayout();
    userTypeLayout->setSpacing(10);
    userTypeLayout->setContentsMargins(0, 0, 0, 0);

    QString buttonStyle = R"(
        QPushButton {
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 500;
            border-radius: 4px;
            border: 1px solid #bdc3c7;
            background-color: white;
            color: #34495e;
        }
        QPushButton:hover {
            background-color: #ecf0f1;
            border-color: #95a5a6;
        }
        QPushButton:checked {
            background-color: #3498db;
            color: white;
            border-color: #2980b9;
        }
    )";

    userButton = new QPushButton("普通用户", this);
    userButton->setCheckable(true);
    userButton->setChecked(true);
    userButton->setStyleSheet(buttonStyle);

    adminButton = new QPushButton("管理员", this);
    adminButton->setCheckable(true);
    adminButton->setStyleSheet(buttonStyle);

    userTypeGroup = new QButtonGroup(this);
    userTypeGroup->addButton(userButton, 0);
    userTypeGroup->addButton(adminButton, 1);
    selectedUserType = "user";

    connect(userTypeGroup, &QButtonGroup::buttonClicked, this, [this](QAbstractButton*) {
        onUserTypeSelected();
    });

    userTypeLayout->addWidget(userButton);
    userTypeLayout->addWidget(adminButton);
    userTypeLayout->addStretch();
    formMainLayout->addLayout(userTypeLayout);

    // 用户名
    QLabel* usernameLabel = new QLabel("用户名:", this);
    usernameLabel->setStyleSheet("color: #2c3e50; font-weight: 600; font-size: 13px;");
    formMainLayout->addWidget(usernameLabel);
    
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("请输入用户名");
    usernameEdit->setStyleSheet(
        "QLineEdit {"
        "   padding: 8px 12px;"
        "   border: 1px solid #bdc3c7;"
        "   border-radius: 4px;"
        "   background-color: white;"
        "   font-size: 13px;"
        "   color: #2c3e50;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #3498db;"
        "   border-width: 2px;"
        "}"
    );
    formMainLayout->addWidget(usernameEdit);

    // 密码
    QLabel* passwordLabel = new QLabel("密码:", this);
    passwordLabel->setStyleSheet("color: #2c3e50; font-weight: 600; font-size: 13px;");
    formMainLayout->addWidget(passwordLabel);
    
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("请输入密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(
        "QLineEdit {"
        "   padding: 8px 12px;"
        "   border: 1px solid #bdc3c7;"
        "   border-radius: 4px;"
        "   background-color: white;"
        "   font-size: 13px;"
        "   color: #2c3e50;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #3498db;"
        "   border-width: 2px;"
        "}"
    );
    formMainLayout->addWidget(passwordEdit);

    mainLayout->addWidget(formFrame);

    // ===== 按钮区域 =====
    QFrame* buttonFrame = new QFrame(this);
    buttonFrame->setStyleSheet("QFrame { background-color: white; }");
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonFrame);
    buttonLayout->setSpacing(10);
    buttonLayout->setContentsMargins(30, 15, 30, 20);
    buttonLayout->addStretch();
    
    QPushButton* cancelBtn = new QPushButton("取消", this);
    cancelBtn->setStyleSheet(
        "QPushButton {"
        "   padding: 8px 20px;"
        "   font-weight: 500;"
        "   font-size: 13px;"
        "   border-radius: 4px;"
        "   background-color: white;"
        "   color: #34495e;"
        "   border: 1px solid #bdc3c7;"
        "}"
        "QPushButton:hover {"
        "   background-color: #ecf0f1;"
        "}"
    );
    
    QPushButton* loginBtn = new QPushButton("登录", this);
    loginBtn->setStyleSheet(
        "QPushButton {"
        "   padding: 8px 28px;"
        "   font-weight: 600;"
        "   font-size: 13px;"
        "   border-radius: 4px;"
        "   background-color: #3498db;"
        "   color: white;"
        "   border: none;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #21618c;"
        "}"
    );
    
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(loginBtn);
    mainLayout->addWidget(buttonFrame);
    
    // 连接按钮信号
    connect(loginBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    // 回车触发登录
    connect(passwordEdit, &QLineEdit::returnPressed, this, &QDialog::accept);

    // 焦点
    usernameEdit->setFocus();

    // 居中显示
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        move((screenGeometry.width() - width()) / 2, 
             (screenGeometry.height() - height()) / 2);
    }
}

LoginDialog::~LoginDialog() {
    // 清理资源
}

void LoginDialog::onUserTypeSelected() {
    selectedUserType = (userTypeGroup->checkedId() == 0) ? "user" : "admin";
}

QString LoginDialog::getUsername() const {
    return usernameEdit->text();
}

QString LoginDialog::getPassword() const {
    return passwordEdit->text();
}

QString LoginDialog::getUserType() const {
    return selectedUserType;
}

void LoginDialog::applyBackground() {
    hasCustomLoginBackground = false;
    loginBackgroundPixmap = QPixmap();
    QString imagePath = AppSettings::instance().loginBackgroundPath();
    if (!imagePath.isEmpty() && QFile::exists(imagePath)) {
        QPixmap pix(imagePath);
        if (!pix.isNull()) {
            loginBackgroundPixmap = pix;
            hasCustomLoginBackground = true;
        }
    }
    updateBackgroundBrush();
}

void LoginDialog::updateBackgroundBrush() {
    QPalette pal = palette();
    if (hasCustomLoginBackground && !loginBackgroundPixmap.isNull()) {
        QPixmap scaled = loginBackgroundPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        pal.setBrush(QPalette::Window, scaled);
        setAutoFillBackground(true);
    } else {
        pal.setBrush(QPalette::Window, QBrush());
        setAutoFillBackground(false);
    }
    setPalette(pal);
}

void LoginDialog::resizeEvent(QResizeEvent* event) {
    QDialog::resizeEvent(event);
    if (hasCustomLoginBackground) {
        updateBackgroundBrush();
    }
}
