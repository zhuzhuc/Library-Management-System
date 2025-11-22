#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>

class AddUserDialog : public QDialog {
    Q_OBJECT
public:
    AddUserDialog(QWidget* parent=nullptr);
    QString getType() const;
    QString getId() const;
    QString getName() const;
    QString getDept() const;
    QString getExtra() const; // major or title
    int getLimit() const;
    QString getPassword() const;

private:
    QComboBox* typeBox;
    QLineEdit* idEdit;
    QLineEdit* nameEdit;
    QLineEdit* deptEdit;
    QLineEdit* extraEdit;
    QLineEdit* passwordEdit;
    QSpinBox* limitSpin;
};
