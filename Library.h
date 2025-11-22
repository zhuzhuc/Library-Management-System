#ifndef LIBRARY_H
#define LIBRARY_H

#include <string>
#include <vector>
#include <map>
#include "Book.h"
// #include "Borrower.h"
class Borrower; // 前向声明 

class Library {
private:
    std::string libraryName;     // 图书馆名称
    std::string location;        // 图书馆位置
        std::vector<Book> books;     // 图书集合 (store by value)
    std::vector<int> borrowedBookIds; // 已借出图书ID
    std::vector<Borrower*> borrowers; // 借阅人列表（基类指针）
    int totalBooks;              // 图书总数
    int availableBooks;          // 可借图书数

public: 
    Library();
    Library(const std::string& name, const std::string& location);
    ~Library();
    
    // 图书管理功能
    void addBook(const Book& book);
    bool removeBook(int bookId);
    Book* findBookById(int bookId);
    Book* findBookByTitle(const std::string& title);
    std::vector<Book*> findBooksByCategory(const std::string& category);
    std::vector<Book*> findBooksByAuthor(const std::string& author);
    
    // 借阅管理功能
    bool lendBook(int bookId);
    bool receiveBook(int bookId);
    bool isBookAvailable(int bookId);
    
    // 借阅人管理
    void addBorrower(Borrower* borrower);
    void removeBorrower(const std::string& borrowerId);
    Borrower* findBorrowerById(const std::string& borrowerId);
    
    // 信息展示功能
    void displayAllBooks() const;
    void displayAvailableBooks() const;
    void displayBorrowedBooks() const;
    void displayLibraryInfo() const;
    void displayStatistics() const;
    
    // 初始化功能
    void initializeWithSampleBooks();
    
    // 工具方法
    void updateStatistics();
    
    // 获取器
    std::string getLibraryName() const { return libraryName; }
    std::string getLocation() const { return location; }
    int getTotalBooks() const { return totalBooks; }
    int getAvailableBooks() const { return availableBooks; }
    
    // expose collections for saving/loading
    const std::vector<Book>& getBooks() const { return books; }
    // non-const access so callers (e.g. GUI controller) can obtain stable pointers to internal Book objects
    std::vector<Book>& getBooks() { return books; }
    const std::vector<Borrower*>& getBorrowers() const { return borrowers; }
    
    // setters for loading
    void setBooks(const std::vector<Book>& newBooks);
    void setBorrowers(const std::vector<Borrower*>& newBorrowers);
    
    // 设置器
    void setLibraryName(const std::string& name) { libraryName = name; }
    void setLocation(const std::string& loc) { location = loc; }
};

#endif // LIBRARY_H