#pragma once

#include <QDialog>
#include <QTableView>
#include <QStandardItemModel>
#include <QLabel>

class LibraryController;
class QProgressBar;

class BookDetailDialog : public QDialog {
    Q_OBJECT
public:
    explicit BookDetailDialog(int bookId, LibraryController* controller, QWidget* parent = nullptr);

private:
    void refreshData();
    QLabel* createValueLabel(const QString& text);
    
    int bookId;
    LibraryController* controller;
    
    // info labels
    QLabel* idValueLabel;
    QLabel* titleValueLabel;
    QLabel* authorValueLabel;
    QLabel* categoryValueLabel;
    QLabel* isbnValueLabel;
    QLabel* availabilityValueLabel;
    QLabel* totalCopiesValueLabel;
    QLabel* borrowedCountValueLabel;
    QProgressBar* availabilityProgressBar;
    QLabel* categoryChipLabel;
    QLabel* statusChipLabel;
    
    QTableView* borrowerTableView;
    QStandardItemModel* borrowerModel;
    QLabel* statsLabel;
};
