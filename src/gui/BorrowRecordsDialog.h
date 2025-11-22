#pragma once

#include <QDialog>
#include <QTableView>
#include <QStandardItemModel>
#include <QLabel>

class LibraryController;

class BorrowRecordsDialog : public QDialog {
    Q_OBJECT
public:
    explicit BorrowRecordsDialog(const QString& borrowerId, const QString& borrowerName, 
                                LibraryController* controller, QWidget* parent = nullptr);

private:
    void refreshData();
    QString borrowerId;
    QString borrowerName;
    LibraryController* controller;
    QTableView* tableView;
    QStandardItemModel* model;
    QLabel* statsLabel;
    QLabel* recordsChipLabel;
    QLabel* borrowedChipLabel;
    QLabel* returnedChipLabel;
};
