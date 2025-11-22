#ifndef BORROWER_H
#define BORROWER_H

#include <string>
#include <vector>
#include <deque>
#include "Book.h"
class Library;

class Borrower {
protected:
    std::string id;         // ID
    std::string name;       // 姓名
    std::string department; // 院系
    int maxBorrowLimit;     // 最大借阅数量
    int currentBorrowCount; // 当前借阅数量
    std::vector<int> borrowedBookIds; // 已借阅图书ID
    std::deque<std::string> borrowHistory; // 历史记录

public:
    Borrower() : id(""), name(""), department(""), maxBorrowLimit(0), currentBorrowCount(0) {}
    Borrower(const std::string& id, const std::string& name, const std::string& dept, int limit)
        : id(id), name(name), department(dept), maxBorrowLimit(limit), currentBorrowCount(0) {}
    
    virtual ~Borrower() {}
    
    // 虚函数实现多态
    virtual bool borrowBookFromLibrary(Library& library, int bookId);
    virtual bool returnBookToLibrary(Library& library, int bookId);
    virtual void displayInfo() const;
    virtual void displayBorrowedBooks(Library& library) const;
    virtual void displayBorrowHistory() const;
    
    // 强制派生类
    virtual std::string getType() const = 0;
    
    // 通用方法
    bool canBorrowMore() const { return currentBorrowCount < maxBorrowLimit; }
    bool hasBorrowedBook(int bookId) const;
    void addToBorrowHistory(const std::string& record);
    
    // 获取器
    std::string getId() const { return id; }
    std::string getName() const { return name; }
    std::string getDepartment() const { return department; }
    int getMaxBorrowLimit() const { return maxBorrowLimit; }
    int getCurrentBorrowCount() const { return currentBorrowCount; }
    

    void setId(const std::string& newId) { id = newId; }
    void setName(const std::string& newName) { name = newName; }
    void setDepartment(const std::string& newDept) { department = newDept; }
    void setMaxBorrowLimit(int limit) { maxBorrowLimit = limit; }
    
    // 新增方法：用于派生类操作借阅记录
    void addBorrowedBookId(int bookId) { borrowedBookIds.push_back(bookId); }
    void incrementBorrowCount() { currentBorrowCount++; }
    void decrementBorrowCount() { if (currentBorrowCount > 0) currentBorrowCount--; }
    // void setId(const std::string& newId) { id = newId; }
    
//获取借阅记录容器的引用
    const std::vector<int>& getBorrowedBookIds() const { return borrowedBookIds; }
}; // class Borrower

#endif // BORROWER_H