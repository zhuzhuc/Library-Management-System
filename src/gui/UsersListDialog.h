#pragma once

#include <QDialog>
#include <QTableView>
#include <QStandardItemModel>

class LibraryController;

class UsersListDialog : public QDialog {
    Q_OBJECT
public:
    explicit UsersListDialog(LibraryController* controller, QWidget* parent = nullptr);

private slots:
    void showUserBorrowRecords(const QModelIndex& index);

private:
    void refreshData();
    LibraryController* controller;
    QTableView* tableView;
    QStandardItemModel* model;
};

