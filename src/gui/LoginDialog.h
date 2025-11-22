#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QButtonGroup>
#include <QGraphicsDropShadowEffect>
#include <QPixmap>

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget* parent = nullptr);
    ~LoginDialog();
    
    // 禁用复制构造函数和赋值操作符
    LoginDialog(const LoginDialog&) = delete;
    LoginDialog& operator=(const LoginDialog&) = delete;
    
    QString getUsername() const;
    QString getPassword() const;
    QString getUserType() const;

private slots:
    void onUserTypeSelected();

private:
    void applyBackground();
    void updateBackgroundBrush();
    void resizeEvent(QResizeEvent* event) override;

    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QPushButton* userButton;
    QPushButton* adminButton;
    QButtonGroup* userTypeGroup;
    QString selectedUserType;
    QGraphicsDropShadowEffect* shadowEffect;
    QPixmap loginBackgroundPixmap;
    bool hasCustomLoginBackground = false;
};

#endif // LOGINDIALOG_H
