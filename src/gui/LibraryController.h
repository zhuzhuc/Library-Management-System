#pragma once

#include <vector>
#include <QObject>
#include <memory>

class Book;
class Library;

namespace db {
    class DBManager;
}

class LibraryController : public QObject {
    Q_OBJECT
public:
    LibraryController(QObject* parent=nullptr);
    ~LibraryController();

    std::vector<Book*> allBooks();
    Book* getBookById(int id);
    std::vector<Book> recommendBooks(int limit = 10);
    bool borrowBook(int id, const std::string& borrowerId, int borrowDays = 7);
    bool returnBook(int id, const std::string& borrowerId);
    void loadFromFiles(); // deprecated, use loadFromDatabase
    void saveToFiles();   // deprecated, use saveToDatabase
    void loadFromDatabase();
    void saveToDatabase();
    void addBook(const Book& book);
    void removeBook(int bookId);
    void addBorrower(class Borrower* borrower);
    void removeBorrower(const std::string& borrowerId);
    
    bool isDatabaseConnected() const;
    db::DBManager* getDBManager() { return dbManager.get(); }

signals:
    void libraryChanged();

private:
    Library* lib;
    std::unique_ptr<db::DBManager> dbManager;
    void initializeDatabase();
    void ensureBaselineBooks();
};
