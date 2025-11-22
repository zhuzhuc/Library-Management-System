#pragma once

#include <QDialog>
#include <QLineEdit>

class ResetPasswordDialog : public QDialog {
    Q_OBJECT
public:
    ResetPasswordDialog(QWidget* parent=nullptr);
    QString getUsername() const;
    QString getNewPassword() const;

private:
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
};

