#include "BookTableModel.h"
#include "LibraryController.h"
#include "Book.h"
#include <QBrush>

BookTableModel::BookTableModel(LibraryController* ctrl, QObject* parent)
    : QAbstractTableModel(parent), controller(ctrl), useFiltered(false) {
    refresh();
    // connect controller signal to refresh the model when library changes
    if (controller) {
        QObject::connect(controller, &LibraryController::libraryChanged, [this]() {
            this->refresh();
        });
    }
}

int BookTableModel::rowCount(const QModelIndex &/*parent*/) const { 
    return useFiltered ? (int)filteredCache.size() : (int)booksCache.size(); 
}
int BookTableModel::columnCount(const QModelIndex &/*parent*/) const { return 6; }

QVariant BookTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return {};
    
    const std::vector<Book*>& cache = useFiltered ? filteredCache : booksCache;
    if (index.row() < 0 || index.row() >= (int)cache.size()) return {};
    Book* b = cache[index.row()];
    if (!b) return {};

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return b->getBookId();
            case 1: return QString::fromStdString(b->getTitle());
            case 2: return QString::fromStdString(b->getAuthor());
            case 3: return QString::fromStdString(b->getCategory());
            case 4: return QString::fromStdString(b->getIsbn());
            case 5: return QString("%1/%2").arg(b->getAvailableCopies()).arg(b->getTotalCopies());
        }
    } else if (role == Qt::BackgroundRole) {
        if (!b->getIsAvailable()) return QBrush(Qt::lightGray);
    }
    return {};
}

QVariant BookTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return {};
    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return "ID";
            case 1: return "Title";
            case 2: return "Author";
            case 3: return "Category";
            case 4: return "ISBN";
            case 5: return "Available/Total";
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

int BookTableModel::bookIdAtRow(int row) const { 
    const std::vector<Book*>& cache = useFiltered ? filteredCache : booksCache;
    return (row>=0 && row<(int)cache.size())? cache[row]->getBookId() : -1; 
}

void BookTableModel::refresh() {
    beginResetModel();
    booksCache.clear();
    filteredCache.clear();
    useFiltered = false;
    auto books = controller->allBooks();
    for (auto b : books) booksCache.push_back(b);
    endResetModel();
}

void BookTableModel::setFilteredBooks(const std::vector<Book*>& filtered) {
    beginResetModel();
    filteredCache = filtered;
    useFiltered = true;
    endResetModel();
}
