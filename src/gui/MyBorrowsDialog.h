#pragma once

#include <QDialog>
#include <QTableView>
#include <QStandardItemModel>
#include <QLabel>

class LibraryController;

class MyBorrowsDialog : public QDialog {
    Q_OBJECT
public:
    explicit MyBorrowsDialog(const QString& borrowerId, LibraryController* controller, QWidget* parent = nullptr);

private:
    void refreshData();
    QString borrowerId;
    LibraryController* controller;
    QTableView* tableView;
    QStandardItemModel* model;
    QLabel* statsLabel;
    QLabel* totalChipLabel;
    QLabel* activeChipLabel;
    QLabel* overdueChipLabel;
};
