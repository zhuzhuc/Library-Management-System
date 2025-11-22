#include "Library.h"
#include <iostream>
#include <algorithm>
#include "Borrower.h"

Library::Library() : libraryName("默认图书馆"), location("未知位置"), totalBooks(0), availableBooks(0) {}

Library::Library(const std::string& name, const std::string& location)
    : libraryName(name), location(location), totalBooks(0), availableBooks(0) {}

void Library::addBook(const Book& book) {
    books.push_back(book);
    updateStatistics(); 
    std::cout << "图书《" << book.getTitle() << "》已添加到图书馆" << std::endl;
}

bool Library::removeBook(int bookId) {
    auto it = std::find_if(books.begin(), books.end(),
        [bookId](const Book& book) { return book.getBookId() == bookId; });
    
    if (it != books.end()) {
        std::cout << "图书《" << it->getTitle() << "》已从图书馆移除" << std::endl;
        books.erase(it);
        updateStatistics();
        return true;
    }
    
    std::cout << "未找到ID为 " << bookId << " 的图书" << std::endl;
    return false;
}

Book* Library::findBookById(int bookId) {
    auto it = std::find_if(books.begin(), books.end(),
        [bookId](const Book& book) { return book.getBookId() == bookId; });
    
    return (it != books.end()) ? &(*it) : nullptr;
}

Book* Library::findBookByTitle(const std::string& title) {
    auto it = std::find_if(books.begin(), books.end(),
        [&title](const Book& book) { return book.getTitle() == title; });
    
    return (it != books.end()) ? &(*it) : nullptr;
}

std::vector<Book*> Library::findBooksByCategory(const std::string& category) {
    std::vector<Book*> result;
    for (auto& book : books) {
        if (book.getCategory() == category) {
            result.push_back(&book);
        }
    }
    return result;
}

std::vector<Book*> Library::findBooksByAuthor(const std::string& author) {
    std::vector<Book*> result;
    for (auto& book : books) {
        if (book.getAuthor() == author) {
            result.push_back(&book);
        }
    }
    return result;
}

bool Library::lendBook(int bookId) {
    // 断点1：方法开始
    Book* book = findBookById(bookId);
    // 断点2：查找图书后
    if (book && book->getIsAvailable() && 
        std::find(borrowedBookIds.begin(), borrowedBookIds.end(), bookId) == borrowedBookIds.end()) {
        
        bool success = book->borrowBook();
        // 断点3：更新图书状态后
        if (success) {
            borrowedBookIds.push_back(bookId);
            availableBooks--;
            // 错误位置：忘记更新availableBooks计数器
            std::cout << "图书《" << book->getTitle() << "》借阅成功" << std::endl;
            return true;
        }
    }
    std::cout << "图书借阅失败" << std::endl;
    return false;
}

bool Library::receiveBook(int bookId) {
    Book* book = findBookById(bookId);
    if (book != nullptr) {
        auto it = std::find(borrowedBookIds.begin(), borrowedBookIds.end(), bookId);
        if (it != borrowedBookIds.end()) {
            if (book->returnBook()) {
                borrowedBookIds.erase(it);
                updateStatistics();
                return true;
            }
        }
    }
    return false;
}

bool Library::isBookAvailable(int bookId) {
    Book* book = findBookById(bookId);
    return (book != nullptr) && book->getIsAvailable();
}

void Library::displayAllBooks() const {
    std::cout << "\n=== " << libraryName << " 所有图书 ===" << std::endl;
    std::cout << "图书馆位置: " << location << std::endl;
    std::cout << "图书总数: " << totalBooks << std::endl;
    std::cout << "可借图书: " << availableBooks << std::endl << std::endl;
    
    if (books.empty()) {
        std::cout << "图书馆暂无图书" << std::endl;
        return;
    }
    
    for (const auto& book : books) {
        book.displayBookInfo();
        std::cout << "-_-_-_-_-_-_-_-__" << std::endl;
    }
}

void Library::displayAvailableBooks() const {
    std::cout << "\n=== 可借图书列表 ===" << std::endl;
    int count = 0;
    for (const auto& book : books) {
        if (book.getIsAvailable()) {
            book.displayBookInfo();
            std::cout << "-_-_-_-_-_-_-_-__" << std::endl;
            count++;
        }
    }
    if (count == 0) {
        std::cout << "暂无可借图书" << std::endl;
    }
}

void Library::displayBorrowedBooks() const {
    std::cout << "\n-_-_-_-_-_-_-_-__ 已借出图书列表 ==-_-_-_-_-_-_-_-__" << std::endl;
    if (borrowedBookIds.empty()) {
        std::cout << "暂无借出图书" << std::endl;
        return;
    }
    
    for (int bookId : borrowedBookIds) {
        const Book* book = const_cast<Library*>(this)->findBookById(bookId);
        if (book != nullptr) {
            std::cout << "ID: " << book->getBookId() 
                      << ", 书名: " << book->getTitle() 
                      << ", 作者: " << book->getAuthor() << std::endl;
        }
    }
}

void Library::displayLibraryInfo() const {
    std::cout << "\n-_-_-_-_-_-_-_-__ 图书馆信息 ==-_-_-_-_-_-_-_-__" << std::endl;
    std::cout << "图书馆名称: " << libraryName << std::endl;
    std::cout << "图书馆位置: " << location << std::endl;
    std::cout << "图书总数: " << totalBooks << std::endl;
    std::cout << "可借图书数: " << availableBooks << std::endl;
    std::cout << "已借出图书数: " << borrowedBookIds.size() << std::endl;
}

void Library::displayStatistics() const {
    displayLibraryInfo();
    
    // 按分类统计
    std::cout << "\n按分类统计:" << std::endl;
    std::vector<std::string> categories;
    for (const auto& book : books) {
        std::string category = book.getCategory();
        if (std::find(categories.begin(), categories.end(), category) == categories.end()) {
            categories.push_back(category);
        }
    }
    
    for (const auto& category : categories) {
        int count = 0;
        for (const auto& book : books) {
            if (book.getCategory() == category) {
                count++;
            }
        }
        std::cout << category << ": " << count << " 本" << std::endl;
    }
}

void Library::initializeWithSampleBooks() {
    addBook(Book(1, "C++程序设计", "谭浩强", "978-7-302-12345-6", "计算机", 3));
    addBook(Book(2, "数据结构", "严蔚敏", "978-7-302-23456-7", "计算机", 2));
    addBook(Book(3, "算法导论", "Thomas H. Cormen", "978-0-262-03384-8", "计算机", 1));
    addBook(Book(4, "红楼梦", "曹雪芹", "978-7-020-00001-1", "文学", 5));
    addBook(Book(5, "西游记", "吴承恩", "978-7-020-00002-2", "文学", 3));
    addBook(Book(6, "C++面向对象设计与分析", "zzc", "5120213235456u5643", "计算机", 100));
    addBook(Book(7, "Go语言中的并发编程", "zzc", "5120213235456u5643", "计算机", 100));
    addBook(Book(8, "三体", "刘慈欣", "978-7-5366-9293-2", "科幻", 6));
    addBook(Book(9, "明朝那些事儿", "当年明月", "978-7-8023-1863-6", "历史", 4));
    addBook(Book(10, "人类简史", "尤瓦尔·赫拉利", "978-7-5442-9236-0", "通识", 5));
    addBook(Book(11, "原则", "Ray Dalio", "978-7-5086-8279-2", "管理", 4));
    addBook(Book(12, "设计模式", "Erich Gamma", "978-7-121-15535-2", "计算机", 3));
    addBook(Book(13, "深入理解计算机系统", "Randal E. Bryant", "978-7-121-15558-1", "计算机", 4));
    addBook(Book(14, "机器学习实战", "Peter Harrington", "978-7-121-32504-6", "人工智能", 3));
    addBook(Book(15, "Python编程：从入门到实践", "Eric Matthes", "978-7-115-42989-9", "计算机", 5));
    addBook(Book(16, "Rust权威指南", "Jim Blandy", "978-7-115-51093-1", "计算机", 3));
    addBook(Book(17, "经济学原理", "N. Gregory Mankiw", "978-7-300-25892-6", "经济", 4));
    addBook(Book(18, "乌合之众", "古斯塔夫·勒庞", "978-7-5356-5923-6", "社会心理", 3));
    addBook(Book(19, "小王子", "安东尼·德·圣-埃克苏佩里", "978-7-5448-0820-9", "文学", 6));
    addBook(Book(20, "解忧杂货店", "东野圭吾", "978-7-5447-6510-3", "文学", 5));
    addBook(Book(21, "深度学习", "Ian Goodfellow", "978-7-121-29829-6", "人工智能", 4));
    addBook(Book(22, "Clean Code", "Robert C. Martin", "978-7-121-15536-9", "软件工程", 5));
    addBook(Book(23, "JavaScript高级程序设计", "Nicholas C. Zakas", "978-7-121-38868-2", "前端开发", 5));
    addBook(Book(24, "React进阶实践指南", "程墨", "978-7-121-38101-0", "前端开发", 3));
    addBook(Book(25, "创新者的窘境", "Clayton M. Christensen", "978-7-300-13892-1", "管理", 4));
    addBook(Book(26, "从0到1", "Peter Thiel", "978-7-115-38904-9", "创业", 4));
    addBook(Book(27, "设计中的设计", "原研哉", "978-7-5321-4374-6", "设计", 6));
    addBook(Book(28, "The Pragmatic Programmer", "Andrew Hunt", "978-0-201-61622-4", "软件工程", 4));
    addBook(Book(29, "时间简史", "Stephen Hawking", "978-7-5619-1855-3", "科普", 5));
    addBook(Book(30, "百年孤独", "Gabriel García Márquez", "978-7-5321-4061-5", "文学", 6));
    addBook(Book(31, "未来简史", "尤瓦尔·赫拉利", "978-7-5442-9480-7", "通识", 4));
    addBook(Book(32, "浪潮之巅", "吴军", "978-7-121-15589-4", "科技", 5));
    addBook(Book(33, "大数据时代", "Viktor Mayer-Schönberger", "978-7-121-25481-8", "数据", 3));
    addBook(Book(34, "黑天鹅", "Nassim Nicholas Taleb", "978-7-5062-6355-0", "经济", 4));
}

void Library::updateStatistics() {
    totalBooks = books.size();
    availableBooks = 0;
    for (const auto& book : books) {
        if (book.getIsAvailable()) {
            availableBooks++;
        }
    }
}

void Library::setBooks(const std::vector<Book>& newBooks) {
    books = newBooks;
    updateStatistics();
}

void Library::setBorrowers(const std::vector<Borrower*>& newBorrowers) {
    // delete old borrowers
    for (auto b : borrowers) delete b;
    borrowers = newBorrowers;
}

// 完成借阅人管理方法的实现
void Library::addBorrower(Borrower* borrower) {
    if (findBorrowerById(borrower->getId()) == nullptr) {
        borrowers.push_back(borrower);
        std::cout << borrower->getType() << " " << borrower->getName() << "已添加到图书馆系统" << std::endl;
    } else {
        std::cout << "添加失败：ID为 " << borrower->getId() << " 的借阅人已存在" << std::endl;
    }
}

void Library::removeBorrower(const std::string& borrowerId) {
    auto it = std::find_if(borrowers.begin(), borrowers.end(),
        [&borrowerId](const Borrower* b) { return b->getId() == borrowerId; });
    
    if (it != borrowers.end()) {
        std::cout << (*it)->getType() << " " << (*it)->getName() << "已从系统中移除" << std::endl;
        delete *it; // 释放内存
        borrowers.erase(it);
    } else {
        std::cout << "未找到ID为 " << borrowerId << " 的借阅人" << std::endl;
    }
}

Borrower* Library::findBorrowerById(const std::string& borrowerId) {
    auto it = std::find_if(borrowers.begin(), borrowers.end(),
        [&borrowerId](const Borrower* b) { return b->getId() == borrowerId; });
    
    return (it != borrowers.end()) ? *it : nullptr;
}

// 析构函数释放借阅人内存
Library::~Library() {
    for (auto borrower : borrowers) {
        delete borrower;
    }
    borrowers.clear();
}
