#include "LibraryController.h"
#include "Library.h"
#include "Book.h"
#include "FileManager.h"
#include "src/db/DBManager.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <unordered_set>
#include <vector>

namespace {
std::string envOrDefault(const char* key, const std::string& fallback) {
    const char* value = std::getenv(key);
    return (value && *value) ? std::string(value) : fallback;
}

unsigned envOrDefaultUnsigned(const char* key, unsigned fallback) {
    const char* value = std::getenv(key);
    if (value && *value) {
        try {
            return static_cast<unsigned>(std::stoul(value));
        } catch (...) {
        }
    }
    return fallback;
}
} // namespace

LibraryController::LibraryController(QObject* /*parent*/) {
    lib = new Library("GUI Library", "local");
    dbManager = std::make_unique<db::DBManager>();
    initializeDatabase();
}

LibraryController::~LibraryController() { 
    // Save to database before destruction
    if (dbManager && dbManager->isConnected()) {
        saveToDatabase();
    }
    delete lib; 
}

void LibraryController::initializeDatabase() {
    // Default MySQL connection settings (can be overridden via env variables)
    const std::string host = envOrDefault("LIBRARY_DB_HOST", "127.0.0.1");
    unsigned port = envOrDefaultUnsigned("LIBRARY_DB_PORT", 3306);
    const std::string user = envOrDefault("LIBRARY_DB_USER", "root");
    const std::string password = envOrDefault("LIBRARY_DB_PASSWORD", "zzcNB123");  // MySQL password
    const std::string dbname = envOrDefault("LIBRARY_DB_NAME", "library_system");
    
    std::vector<std::string> hostCandidates = {host};
    if (host != "127.0.0.1") {
        hostCandidates.push_back("127.0.0.1");
    }
    
    bool connected = false;
    for (const auto& candidate : hostCandidates) {
        if (dbManager->connect(candidate, port, user, password, dbname)) {
            connected = true;
            break;
        }
    }
    
    if (!connected) {
        std::cerr << "Failed to connect to MySQL database. Using file storage as fallback." << std::endl;
        // Fallback to sample books if DB connection fails
        lib->initializeWithSampleBooks();
        return;
    }
    
    // Create schema if not exists
    if (!dbManager->createSchema()) {
        std::cerr << "Failed to create database schema." << std::endl;
        return;
    }
    
    // Load data from database
    loadFromDatabase();
    ensureBaselineBooks();
    
    // If no books in database, initialize with sample books
    if (lib->getBooks().empty()) {
        lib->initializeWithSampleBooks();
        saveToDatabase();
    }
}

std::vector<Book*> LibraryController::allBooks() {
    std::vector<Book*> res;
    // Return pointers to internal Book objects to avoid copies. Caller must NOT delete these pointers.
    for (auto &b : lib->getBooks()) {
        res.push_back(&b);
    }
    return res;
}

Book* LibraryController::getBookById(int id) {
    for (auto &b : lib->getBooks()) {
        if (b.getBookId() == id) return &b;
    }
    return nullptr;
}

std::vector<Book> LibraryController::recommendBooks(int limit) {
    std::vector<Book> recommendations;
    std::vector<Book*> allBooks = this->allBooks();
    
    if (allBooks.empty()) {
        return recommendations;
    }
    
    // 计算每本书的借阅次数（受欢迎程度）
    std::map<int, int> bookBorrowCount;
    
    // 如果有数据库连接，从借阅记录中获取借阅统计
    if (dbManager && dbManager->isConnected()) {
        std::vector<std::map<std::string, std::string>> borrowRecords;
        if (dbManager->getAllBorrowRecords(borrowRecords)) {
            for (const auto& record : borrowRecords) {
                auto it = record.find("book_id");
                if (it != record.end()) {
                    int bookId = std::stoi(it->second);
                    bookBorrowCount[bookId]++;
                }
            }
        }
    }
    
    // 创建带评分的书籍列表
    struct BookScore {
        Book* book;
        int score;
    };
    
    std::vector<BookScore> scoredBooks;
    for (Book* book : allBooks) {
        int score = 0;
        
        // 基础分数：可借数量越多，分数越高
        score += book->getAvailableCopies() * 10;
        
        // 受欢迎程度：借阅次数越多，分数越高
        score += bookBorrowCount[book->getBookId()] * 5;
        
        // 如果可借，额外加分
        if (book->getIsAvailable()) {
            score += 20;
        }
        
        scoredBooks.push_back({book, score});
    }
    
    // 按分数降序排序
    std::sort(scoredBooks.begin(), scoredBooks.end(), 
              [](const BookScore& a, const BookScore& b) {
                  return a.score > b.score;
              });
    
    // 优先选择可借的书籍，然后选择其他书籍
    std::vector<Book> availableBooks;
    std::vector<Book> otherBooks;
    
    for (const auto& item : scoredBooks) {
        if (item.book->getIsAvailable()) {
            availableBooks.push_back(*item.book);
        } else {
            otherBooks.push_back(*item.book);
        }
    }
    
    // 先添加可借书籍，再添加其他书籍，直到达到限制
    int count = 0;
    for (const auto& book : availableBooks) {
        if (count >= limit) break;
        recommendations.push_back(book);
        count++;
    }
    
    for (const auto& book : otherBooks) {
        if (count >= limit) break;
        recommendations.push_back(book);
        count++;
    }
    
    return recommendations;
}

bool LibraryController::borrowBook(int id, const std::string& borrowerId, int borrowDays) {
    bool ok = lib->lendBook(id);
    if (ok) {
        if (dbManager && dbManager->isConnected()) {
            Book* book = lib->findBookById(id);
            if (book) {
                dbManager->upsertBook(*book);
                // Create borrow record
                dbManager->createBorrowRecord(borrowerId, id, borrowDays);
            }
        }
        emit libraryChanged();
    }
    return ok;
}

bool LibraryController::returnBook(int id, const std::string& borrowerId) {
    // 先检查数据库中是否有该借阅记录
    if (dbManager && dbManager->isConnected()) {
        std::vector<std::map<std::string, std::string>> records;
        if (dbManager->getActiveBorrowRecordsByBorrower(borrowerId, records)) {
            bool hasRecord = false;
            for (const auto& record : records) {
                if (record.find("book_id") != record.end()) {
                    try {
                        if (std::stoi(record.at("book_id")) == id) {
                            hasRecord = true;
                            break;
                        }
                    } catch (...) {
                        // 转换失败，继续检查下一条
                        continue;
                    }
                }
            }
            if (!hasRecord) {
                return false; // 该用户没有借阅过这本书
            }
        }
    }
    
    // 更新数据库记录
    bool dbOk = false;
        if (dbManager && dbManager->isConnected()) {
        dbOk = dbManager->returnBorrowRecord(id, borrowerId);
        if (dbOk) {
            // 更新图书状态
            Book* book = lib->findBookById(id);
            if (book) {
                dbManager->upsertBook(*book);
            }
        }
    }
    
    // 更新内存中的图书状态
    bool ok = lib->receiveBook(id);
    
    if (ok || dbOk) {
        emit libraryChanged();
        return true;
    }
    return false;
}

void LibraryController::loadFromFiles() {
    std::vector<Book> books;
    FileManager::loadBooksFromFile(books, "books.json");
    lib->setBooks(books);
    emit libraryChanged();
}

void LibraryController::saveToFiles() {
    FileManager::saveBooksToFile(lib->getBooks(), "books.json");
    emit libraryChanged();
}

void LibraryController::loadFromDatabase() {
    if (!dbManager || !dbManager->isConnected()) {
        return;
    }
    
    std::vector<Book> books;
    std::vector<Borrower*> borrowers;
    
    if (dbManager->loadBooks(books)) {
        lib->setBooks(books);
    }
    
    if (dbManager->loadBorrowers(borrowers)) {
        lib->setBorrowers(borrowers);
    }
    
    lib->updateStatistics();
    emit libraryChanged();
}

void LibraryController::ensureBaselineBooks() {
    if (!dbManager || !dbManager->isConnected()) {
        return;
    }
    
    std::unordered_set<int> existingIds;
    for (const auto& book : lib->getBooks()) {
        existingIds.insert(book.getBookId());
    }
    
    Library seedLibrary("Baseline", "default");
    seedLibrary.initializeWithSampleBooks();
    const auto& sampleBooks = seedLibrary.getBooks();
    bool added = false;
    for (const auto& book : sampleBooks) {
        if (existingIds.insert(book.getBookId()).second) {
            lib->addBook(book);
            added = true;
        }
    }
    
    if (added) {
        saveToDatabase();
    }
}

void LibraryController::saveToDatabase() {
    if (!dbManager || !dbManager->isConnected()) {
        return;
    }
    
    dbManager->saveBooks(lib->getBooks());
    dbManager->saveBorrowers(lib->getBorrowers());
}

void LibraryController::addBook(const Book& book) {
    lib->addBook(book);
    if (dbManager && dbManager->isConnected()) {
        dbManager->upsertBook(book);
    }
    emit libraryChanged();
}

void LibraryController::removeBook(int bookId) {
    if (lib->removeBook(bookId)) {
        if (dbManager && dbManager->isConnected()) {
            dbManager->removeBook(bookId);
        }
        emit libraryChanged();
    }
}

void LibraryController::addBorrower(Borrower* borrower) {
    lib->addBorrower(borrower);
    if (dbManager && dbManager->isConnected()) {
        dbManager->upsertBorrower(borrower);
    }
    emit libraryChanged();
}

void LibraryController::removeBorrower(const std::string& borrowerId) {
    // Check if borrower exists before removal
    bool existed = (lib->findBorrowerById(borrowerId) != nullptr);
    lib->removeBorrower(borrowerId);
    
    // Only update database if borrower was actually removed
    if (existed) {
        if (dbManager && dbManager->isConnected()) {
            dbManager->removeBorrower(borrowerId);
        }
        emit libraryChanged();
    }
}

bool LibraryController::isDatabaseConnected() const {
    return dbManager && dbManager->isConnected();
}
