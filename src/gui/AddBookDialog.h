#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>

class AddBookDialog : public QDialog {
    Q_OBJECT
public:
    AddBookDialog(QWidget* parent=nullptr);
    int getId() const;
    QString getTitleStr() const;
    QString getAuthor() const;
    QString getIsbn() const;
    QString getCategory() const;
    int getCopies() const;

private:
    QLineEdit* idEdit;
    QLineEdit* titleEdit;
    QLineEdit* authorEdit;
    QLineEdit* isbnEdit;
    QLineEdit* categoryEdit;
    QSpinBox* copiesSpin;
};
