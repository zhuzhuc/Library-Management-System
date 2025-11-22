#include "Book.h"

Book::Book() : bookId(0), title(""), author(""), isbn(""), category(""), 
               totalCopies(0), availableCopies(0), isAvailable(false) {}

Book::Book(int id, const std::string& title, const std::string& author, 
           const std::string& isbn, const std::string& category, int copies)
    : bookId(id), title(title), author(author), isbn(isbn), category(category),
      totalCopies(copies), availableCopies(copies) {
    updateAvailability();
}

bool Book::borrowBook() {
    if (availableCopies > 0) {
        availableCopies--;
        updateAvailability();
        return true;
    }
    return false;
}

bool Book::returnBook() {
    if (availableCopies < totalCopies) {
        availableCopies++;
        updateAvailability();
        return true;
    }
    return false;
}

void Book::displayBookInfo() const {
    std::cout << "图书ID: " << bookId << std::endl;
    std::cout << "书名: " << title << std::endl;
    std::cout << "作者: " << author << std::endl;
    std::cout << "ISBN: " << isbn << std::endl;
    std::cout << "分类: " << category << std::endl;
    std::cout << "总数量: " << totalCopies << std::endl;
    std::cout << "可借数量: " << availableCopies << std::endl;
    std::cout << "状态: " << (isAvailable ? "可借" : "不可借") << std::endl;
}

void Book::displayDetailedInfo() const {
    displayBookInfo();
    std::cout << "详细信息已显示" << std::endl;
}

void Book::updateAvailability() {
    isAvailable = (availableCopies > 0);
}