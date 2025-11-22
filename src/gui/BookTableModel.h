#pragma once
#include <QAbstractTableModel>
#include <vector>

class LibraryController;
class Book;

class BookTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    BookTableModel(LibraryController* ctrl, QObject* parent=nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    int bookIdAtRow(int row) const;
    void refresh();
    void setFilteredBooks(const std::vector<Book*>& filtered);

private:
    LibraryController* controller;
    std::vector<Book*> booksCache; // cache pointers to Library's internal Book objects
    bool useFiltered;
    std::vector<Book*> filteredCache;
};
