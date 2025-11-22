
#include "Borrower.h"
#include "Library.h" 
#include <iostream>
#include <algorithm>
#include <ctime>

bool Borrower::borrowBookFromLibrary(Library& library, int bookId) {
    std::cout << "\n=== " << getType() << "借书流程开始 ===" << std::endl;
    std::cout << getType() << " " << name << " 尝试借阅图书ID: " << bookId << std::endl;
    
    if (!canBorrowMore()) {
        std::cout << "借书失败: " << getType() << " " << name << " 已达到借书上限 (" << maxBorrowLimit << " 本)" << std::endl;
        return false;
    }
    
    if (hasBorrowedBook(bookId)) {
        std::cout << "借书失败: " << getType() << " " << name << " 已借阅过该图书" << std::endl;
        return false;
    }
    
    // 使用图书馆的服务查找图书
    Book* book = library.findBookById(bookId);
    if (book == nullptr) {
        std::cout << "借书失败: 图书馆中未找到ID为 " << bookId << " 的图书" << std::endl;
        return false;
    }
    
    //  检查图书可借状态
    if (!book->getIsAvailable()) {
        std::cout << "借书失败: 图书《" << book->getTitle() << "》暂无可借副本" << std::endl;
        return false;
    }
    
    // 完成借书操作
    if (library.lendBook(bookId)) {
        // 借书记录
        borrowedBookIds.push_back(bookId);
        currentBorrowCount++;
        
        // 借书历史
        std::string record = "借阅《" + book->getTitle() + "》";
        addToBorrowHistory(record);
        
        std::cout << "借书成功: " << getType() << " " << name << " 成功借阅《" << book->getTitle() << "》" << std::endl;
        std::cout << "当前借书数量: " << currentBorrowCount << "/" << maxBorrowLimit << std::endl;
        return true;
    }
    
    std::cout << "借书失败: 图书馆系统错误" << std::endl;
    return false;
}

bool Borrower::returnBookToLibrary(Library& library, int bookId) {
    std::cout << "\n=== " << getType() << "还书流程开始 ===" << std::endl;
    std::cout << getType() << " " << name << " 尝试归还图书ID: " << bookId << std::endl;
    
    if (!hasBorrowedBook(bookId)) {
        std::cout << "还书失败: " << getType() << " " << name << " 未借阅过该图书" << std::endl;
        return false;
    }
    
    Book* book = library.findBookById(bookId);
    if (book == nullptr) {
        std::cout << "还书失败: 图书馆中未找到ID为 " << bookId << " 的图书" << std::endl;
        return false;
    }
    
    // 通过图书馆完成还书操作
    if (library.receiveBook(bookId)) {
        // 使用STL算法查找并删除借书记录
        auto it = std::find(borrowedBookIds.begin(), borrowedBookIds.end(), bookId);
        if (it != borrowedBookIds.end()) {
            borrowedBookIds.erase(it);
            currentBorrowCount--;
            
            // 记录还书历史
            std::string record = "归还《" + book->getTitle() + "》";
            addToBorrowHistory(record);
            
            std::cout << "还书成功: " << getType() << " " << name << " 成功归还《" << book->getTitle() << "》" << std::endl;
            std::cout << "当前借书数量: " << currentBorrowCount << "/" << maxBorrowLimit << std::endl;
            return true;
        }
    }
    
    std::cout << "还书失败: 图书馆系统错误" << std::endl;
    return false;
}

void Borrower::displayInfo() const {
    std::cout << "\n=== " << getType() << "信息 ===" << std::endl;
    std::cout << getType() << "ID: " << id << std::endl;
    std::cout << "姓名: " << name << std::endl;
    std::cout << "院系: " << department << std::endl;
    std::cout << "借书上限: " << maxBorrowLimit << std::endl;
    std::cout << "当前借书数量: " << currentBorrowCount << std::endl;
    std::cout << "剩余可借数量: " << (maxBorrowLimit - currentBorrowCount) << std::endl;
}

void Borrower::displayBorrowedBooks(Library& library) const {
    std::cout << "\n" << getType() << " " << name << " 的借书记录:" << std::endl;
    if (borrowedBookIds.empty()) {
        std::cout << "暂无借书记录" << std::endl;
        return;
    }
    
    // 使用STL容器遍历
    for (int bookId : borrowedBookIds) {
        Book* book = library.findBookById(bookId);
        if (book != nullptr) {
            std::cout << "- ID: " << book->getBookId() 
                      << ", 书名: 《" << book->getTitle() << "》"
                      << ", 作者: " << book->getAuthor() << std::endl;
        }
    }
}

void Borrower::displayBorrowHistory() const {
    std::cout << "\n" << getType() << " " << name << " 的借阅历史:" << std::endl;
    if (borrowHistory.empty()) {
        std::cout << "暂无借阅历史" << std::endl;
        return;
    }
    
    // 使用STL容器遍历
    for (const auto& record : borrowHistory) {
        std::cout << "- " << record << std::endl;
    }
}

bool Borrower::hasBorrowedBook(int bookId) const {
    // 使用STL算法查找
    return std::find(borrowedBookIds.begin(), borrowedBookIds.end(), bookId) != borrowedBookIds.end();
}

void Borrower::addToBorrowHistory(const std::string& record) {
    // 获取当前时间
    time_t now = time(0);
    char* timeStr = ctime(&now);
    std::string timeString(timeStr);
    timeString.pop_back(); // 移除换行符
    
    // 使用deque的特性：在前面插入最新记录
    borrowHistory.push_front(timeString + " - " + record);
    
    // 限制历史记录数量，保持最近20条记录
    if (borrowHistory.size() > 20) {
        borrowHistory.pop_back(); // deque的高效尾部删除
    }
}