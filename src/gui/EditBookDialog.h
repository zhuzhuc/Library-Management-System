#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>

class EditBookDialog : public QDialog {
    Q_OBJECT
public:
    EditBookDialog(QWidget* parent=nullptr);
    void setBookData(int id, const QString& title, const QString& author, 
                     const QString& isbn, const QString& category, int copies);
    
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

