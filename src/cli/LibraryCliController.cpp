#include "src/cli/LibraryCliController.h"

#include "Book.h"
#include "Borrower.h"
#include "FileManager.h"
#include "Library.h"
#include "Student.h"
#include "Teacher.h"
#include "src/cli/BookRecommendationService.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

#ifdef USE_MYSQL
#include "src/db/DBManager.h"
#endif

namespace {

int promptInt(const std::string& message, int defaultValue = 0) {
    while (true) {
        std::cout << message;
        std::string buffer;
        std::getline(std::cin >> std::ws, buffer);
        if (buffer.empty() && defaultValue != 0) return defaultValue;
        std::stringstream ss(buffer);
        int value = 0;
        if (ss >> value) return value;
        std::cout << "请输入有效的数字。" << std::endl;
    }
}

std::string promptLine(const std::string& message, const std::string& defaultValue = {}) {
    std::cout << message;
    std::string buffer;
    std::getline(std::cin >> std::ws, buffer);
    if (buffer.empty()) return defaultValue;
    return buffer;
}

} // namespace

namespace cli {

LibraryCliController::LibraryCliController(Library& library)
    : library_(library) {}

void LibraryCliController::bootstrap() {
#ifdef USE_MYSQL
    if (promptDatabaseConfig() && loadLibraryFromDatabase()) {
        std::cout << "已从数据库加载数据。" << std::endl;
        return;
    }
#endif
    if (!loadLibrary()) {
        initializeFallbackData();
    }
}

void LibraryCliController::run() {
    bool running = true;
    while (running) {
        showMainMenu();
        int choice = promptInt("请选择操作 (1-3): ");
        switch (choice) {
            case 1:
                handleAdminSession();
                break;
            case 2:
                handleUserSession();
                break;
            case 3:
                running = false;
                std::cout << "-_-_-_-_-_-_-_-__ 谢谢使用，再见！_-_-_-_-_-_-_-_-__" << std::endl;
                break;
            default:
                std::cout << "无效的选项，请重新选择！" << std::endl;
        }
    }
}

void LibraryCliController::showMainMenu() const {
    std::cout << "\n/_-_-_-_-_-_-_-__ 图书馆管理系统 __-_-_-_-_-_-_-_/" << std::endl;
    std::cout << "1. 管理员登录" << std::endl;
    std::cout << "2. 用户登录" << std::endl;
    std::cout << "3. 退出系统" << std::endl;
}

void LibraryCliController::showAdminMenu() const {
    std::cout << "\n/_-_-_-_-_-_-_-__ 管理员菜单 __-_-_-_-_-_-_-_/" << std::endl;
    std::cout << "1. 添加图书" << std::endl;
    std::cout << "2. 删除图书" << std::endl;
    std::cout << "3. 查看所有图书" << std::endl;
    std::cout << "4. 查看可借图书" << std::endl;
    std::cout << "5. 查看已借出图书" << std::endl;
    std::cout << "6. 查看图书馆统计信息" << std::endl;
    std::cout << "7. 添加用户" << std::endl;
    std::cout << "8. 删除用户" << std::endl;
    std::cout << "9. 保存数据" << std::endl;
    std::cout << "10. 加载数据" << std::endl;
    std::cout << "0. 返回主菜单" << std::endl;
}

void LibraryCliController::showUserMenu() const {
    std::cout << "\n/_-_-_-_-_-_-_-__ 用户菜单 __-_-_-_-_-_-_-_/" << std::endl;
    std::cout << "1. 查看个人信息" << std::endl;
    std::cout << "2. 查找图书" << std::endl;
    std::cout << "3. 按分类浏览图书" << std::endl;
    std::cout << "4. 借阅图书" << std::endl;
    std::cout << "5. 归还图书" << std::endl;
    std::cout << "6. 查看我的借阅" << std::endl;
    std::cout << "7. 查看借阅历史" << std::endl;
    std::cout << "0. 退出登录" << std::endl;
}

void LibraryCliController::handleAdminSession() {
    if (!authenticateAdmin()) {
        std::cout << "管理员密码错误！" << std::endl;
        return;
    }

    bool adminRunning = true;
    while (adminRunning) {
        showAdminMenu();
        int choice = promptInt("请选择操作 (0-10): ");
        processAdminChoice(choice, adminRunning);
    }
}

void LibraryCliController::handleUserSession() {
    Borrower* user = authenticateUser();
    if (!user) return;

    bool userRunning = true;
    while (userRunning) {
        showUserMenu();
        int choice = promptInt("请选择操作 (0-7): ");
        processUserChoice(user, choice, userRunning);
    }
}

bool LibraryCliController::authenticateAdmin() const {
    std::string password;
    std::cout << "请输入管理员密码: ";
    std::cin >> password;
    return password == "admin123";
}

Borrower* LibraryCliController::authenticateUser() const {
    std::string userId, password;
    std::cout << "请输入用户ID: ";
    std::cin >> userId;
    std::cout << "请输入密码: ";
    std::cin >> password;

    Borrower* currentUser = library_.findBorrowerById(userId);
    if (currentUser) {
        std::string expected = userId.substr(userId.length() > 6 ? userId.length() - 6 : 0);
        if (password == expected) {
            std::cout << "登录成功，欢迎 " << currentUser->getName() << "!" << std::endl;
            return currentUser;
        }
    }
    std::cout << "用户ID或密码错误！" << std::endl;
    return nullptr;
}

void LibraryCliController::processAdminChoice(int choice, bool& keepRunning) {
    switch (choice) {
        case 1:
            promptAndAddBook();
            break;
        case 2:
            promptAndRemoveBook();
            break;
        case 3:
            library_.displayAllBooks();
            break;
        case 4:
            library_.displayAvailableBooks();
            break;
        case 5:
            library_.displayBorrowedBooks();
            break;
        case 6:
            library_.displayStatistics();
            recommendation_.printStatistics();
            break;
        case 7:
            promptAndAddUser();
            break;
        case 8:
            promptAndRemoveUser();
            break;
        case 9:
            saveLibrary();
            break;
        case 10:
            loadLibrary();
            break;
        case 0:
            keepRunning = false;
            std::cout << "已退出管理员模式！" << std::endl;
            break;
        default:
            std::cout << "无效的选项，请重新选择！" << std::endl;
    }
}

void LibraryCliController::processUserChoice(Borrower* user, int choice, bool& keepRunning) {
    switch (choice) {
        case 1:
            user->displayInfo();
            break;
        case 2: {
            std::string keyword = promptLine("请输入要查找的图书名: ");
            Book* book = library_.findBookByTitle(keyword);
            if (book) {
                book->displayDetailedInfo();
            } else {
                std::cout << "未找到匹配的图书。" << std::endl;
            }
            break;
        }
        case 3: {
            std::string category = promptLine("请输入分类关键字: ");
            auto books = library_.findBooksByCategory(category);
            if (books.empty()) {
                std::cout << "该分类暂无图书。" << std::endl;
            } else {
                for (auto* book : books) {
                    book->displayBookInfo();
                }
            }
            break;
        }
        case 4:
            handleBorrowFlow(user);
            break;
        case 5:
            handleReturnFlow(user);
            break;
        case 6:
            user->displayBorrowedBooks(library_);
            break;
        case 7:
            user->displayBorrowHistory();
            break;
        case 0:
            keepRunning = false;
            std::cout << "已退出登录！" << std::endl;
            break;
        default:
            std::cout << "无效的选项，请重新选择！" << std::endl;
    }
}

void LibraryCliController::promptAndAddBook() {
    int id = promptInt("请输入图书ID: ");
    std::string title = promptLine("请输入书名: ");
    std::string author = promptLine("请输入作者: ");
    std::string isbn = promptLine("请输入ISBN: ");
    std::string category = promptLine("请输入分类: ");
    int copies = promptInt("请输入总数量: ");

    Book book(id, title, author, isbn, category, copies);
    library_.addBook(book);
#ifdef USE_MYSQL
    if (dbMode_) {
        syncBookToDatabase(id);
    }
#endif
}

void LibraryCliController::promptAndRemoveBook() {
    int bookId = promptInt("请输入要删除的图书ID: ");
    if (library_.removeBook(bookId)) {
        std::cout << "已删除书籍 " << bookId << std::endl;
#ifdef USE_MYSQL
        if (dbMode_) {
            withDbConnection([&](db::DBManager& mgr) { mgr.removeBook(bookId); });
        }
#endif
    } else {
        std::cout << "未找到该书籍。" << std::endl;
    }
}

void LibraryCliController::promptAndAddUser() {
    int type = promptInt("请选择用户类型 (1-学生, 2-教师): ");
    std::string id = promptLine("请输入用户ID: ");
    std::string name = promptLine("请输入姓名: ");
    std::string department = promptLine("请输入院系: ");
    int limit = promptInt("请输入最大借阅数量: ");

    if (type == 1) {
        std::string major = promptLine("请输入专业: ");
        library_.addBorrower(new Student(id, name, department, major, limit));
    } else {
        std::string title = promptLine("请输入职称: ");
        library_.addBorrower(new Teacher(id, name, department, title, limit));
    }
#ifdef USE_MYSQL
    if (dbMode_) {
        Borrower* borrower = library_.findBorrowerById(id);
        if (borrower) {
            withDbConnection([&](db::DBManager& mgr) { mgr.upsertBorrower(borrower); });
        }
    }
#endif
}

void LibraryCliController::promptAndRemoveUser() {
    std::string borrowerId = promptLine("请输入要删除的用户ID: ");
#ifdef USE_MYSQL
    if (dbMode_) {
        withDbConnection([&](db::DBManager& mgr) { mgr.removeBorrower(borrowerId); });
    }
#endif
    library_.removeBorrower(borrowerId);
}

void LibraryCliController::handleBorrowFlow(Borrower* user) {
    int bookId = promptInt("请输入要借阅的图书ID: ");
    if (user->borrowBookFromLibrary(library_, bookId)) {
        if (Book* book = library_.findBookById(bookId)) {
            recommendation_.recordBorrow(bookId, book->getCategory());
        }
#ifdef USE_MYSQL
        if (dbMode_) {
            syncBookToDatabase(bookId);
        }
#endif
    }
}

void LibraryCliController::handleReturnFlow(Borrower* user) {
    int bookId = promptInt("请输入要归还的图书ID: ");
    if (user->returnBookToLibrary(library_, bookId)) {
#ifdef USE_MYSQL
        if (dbMode_) {
            syncBookToDatabase(bookId);
        }
#endif
    }
}

void LibraryCliController::initializeFallbackData() {
    library_.initializeWithSampleBooks();
    library_.addBorrower(new Student("56666666666", "zzc", "计算机学院", "信息安全", 5));
    library_.addBorrower(new Student("2023002", "李四", "文学院", "汉语言文学", 5));
    library_.addBorrower(new Teacher("T2023001", "王教授", "计算机学院", "教授", 10));
}

bool LibraryCliController::saveLibrary() {
#ifdef USE_MYSQL
    if (dbMode_ && persistLibraryToDatabase()) {
        return true;
    }
#endif
    return saveToFiles();
}

bool LibraryCliController::loadLibrary() {
#ifdef USE_MYSQL
    if (dbMode_) {
        if (loadLibraryFromDatabase()) return true;
        std::cout << "数据库加载失败，尝试读取本地文件..." << std::endl;
    }
#endif
    return loadFromFiles();
}

bool LibraryCliController::saveToFiles() const {
    return FileManager::saveLibrarySnapshot(library_, booksFile_, usersFile_);
}

bool LibraryCliController::loadFromFiles() {
    return FileManager::loadLibrarySnapshot(library_, booksFile_, usersFile_);
}

#ifdef USE_MYSQL

bool LibraryCliController::promptDatabaseConfig() {
    char answer = 'n';
    std::cout << "是否启用数据库模式? (y/n): ";
    std::cin >> answer;
    if (answer != 'y' && answer != 'Y') {
        dbMode_ = false;
        return false;
    }

    DbConfig config;
    config.host = promptLine("DB host (默认 127.0.0.1): ", "127.0.0.1");
    config.port = static_cast<unsigned>(promptInt("DB port (默认 3306): ", 3306));
    config.user = promptLine("DB user: ");
    config.password = promptLine("DB password (可留空): ");
    config.database = promptLine("DB name: ");

    if (config.user.empty() || config.database.empty()) {
        std::cout << "数据库用户名与库名不能为空，取消数据库模式。" << std::endl;
        dbMode_ = false;
        return false;
    }

    db::DBManager mgr;
    if (!mgr.connect(config.host, config.port, config.user, config.password, config.database)) {
        std::cout << "无法连接到数据库，继续使用文件模式。" << std::endl;
        dbMode_ = false;
        return false;
    }
    mgr.createSchema();
    mgr.disconnect();

    dbConfig_ = config;
    dbMode_ = true;
    return true;
}

bool LibraryCliController::withDbConnection(const std::function<void(db::DBManager&)>& fn) const {
    if (!dbMode_ || !dbConfig_) return false;
    db::DBManager mgr;
    const auto& cfg = *dbConfig_;
    if (!mgr.connect(cfg.host, cfg.port, cfg.user, cfg.password, cfg.database)) {
        std::cout << "数据库连接失败。" << std::endl;
        return false;
    }
    mgr.createSchema();
    fn(mgr);
    mgr.disconnect();
    return true;
}

bool LibraryCliController::loadLibraryFromDatabase() {
    bool loaded = false;
    withDbConnection([&](db::DBManager& mgr) {
        std::vector<Book> books;
        std::vector<Borrower*> borrowers;
        if (mgr.loadBooks(books)) {
            library_.setBooks(books);
            loaded = true;
        }
        if (mgr.loadBorrowers(borrowers)) {
            library_.setBorrowers(borrowers);
            loaded = true;
        }
    });
    return loaded;
}

bool LibraryCliController::persistLibraryToDatabase() {
    return withDbConnection([&](db::DBManager& mgr) {
        mgr.saveBooks(library_.getBooks());
        mgr.saveBorrowers(library_.getBorrowers());
    });
}

bool LibraryCliController::syncBookToDatabase(int bookId) {
    Book* book = library_.findBookById(bookId);
    if (!book) return false;
    return withDbConnection([&](db::DBManager& mgr) { mgr.upsertBook(*book); });
}

#endif

} // namespace cli
