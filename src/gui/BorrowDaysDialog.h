#pragma once

#include <QDialog>
#include <QSpinBox>

class BorrowDaysDialog : public QDialog {
    Q_OBJECT
public:
    BorrowDaysDialog(QWidget* parent = nullptr);
    int getDays() const;

private:
    QSpinBox* daysSpin;
};

