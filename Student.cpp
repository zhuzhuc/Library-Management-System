#include "Student.h"
#include <iostream>
#include "Library.h"

void Student::displayInfo() const {
    Borrower::displayInfo(); // 调用基类方法
    std::cout << "专业: " << major << std::endl; // 添加专业信息
}

bool Student::searchBookInLibrary(Library& library, const std::string& title) {
    // 依赖关系：学生依赖图书馆提供查询服务
    
    std::cout << "学生 " << getName() << " 正在图书馆查找图书: 《" << title << "》" << std::endl;
    
    Book* book = library.findBookByTitle(title);
    if (book != nullptr) {
        std::cout << "找到图书:" << std::endl;
        book->displayBookInfo();
        return true;
    } else {
        std::cout << "未找到图书: 《" << title << "》" << std::endl;
        return false;
    }
}

std::vector<Book*> Student::browseBooksByCategory(Library& library, const std::string& category) {
    // 依赖关系：学生依赖图书馆提供分类浏览服务
    
    std::cout << "学生 " << getName() << " 正在浏览 " << category << " 分类的图书" << std::endl;
    
    std::vector<Book*> books = library.findBooksByCategory(category); // STL容器返回值
    
    if (!books.empty()) {
        std::cout << "找到 " << books.size() << " 本 " << category << " 类图书:" << std::endl;
        for (Book* book : books) { // STL容器遍历
            std::cout << "- 《" << book->getTitle() << "》 by " << book->getAuthor();
            std::cout << " [" << (book->getIsAvailable() ? "可借" : "不可借") << "]" << std::endl;
        }
    } else {
        std::cout << "未找到 " << category << " 分类的图书" << std::endl;
    }
    
    return books;
}

// 修复displayStudentInfo方法，使用id代替未定义的studentId
void Student::displayStudentInfo() const {
    std::cout << "\n=== 学生信息 ===" << std::endl;
    std::cout << "学号: " << id << std::endl;  // 使用id而不是studentId
    std::cout << "姓名: " << name << std::endl;
    std::cout << "专业: " << major << std::endl;
    std::cout << "借书上限: " << maxBorrowLimit << std::endl;
    std::cout << "当前借书数量: " << currentBorrowCount << std::endl;
    std::cout << "剩余可借数量: " << (maxBorrowLimit - currentBorrowCount) << std::endl;
}