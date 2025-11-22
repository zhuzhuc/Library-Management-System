#pragma once

#include <functional>
#include <optional>
#include <string>

#include "src/cli/BookRecommendationService.h"

#ifdef USE_MYSQL
namespace db {
class DBManager;
}
#endif

class Library;
class Borrower;
class Book;

namespace cli {

struct DbConfig {
    std::string host = "127.0.0.1";
    unsigned port = 3306;
    std::string user;
    std::string password;
    std::string database;
};

class LibraryCliController {
public:
    explicit LibraryCliController(Library& library);

    void bootstrap();
    void run();

private:
    Library& library_;
    bool dbMode_ = false;
    std::optional<DbConfig> dbConfig_;
    std::string booksFile_ = "books.tsv";
    std::string usersFile_ = "users.tsv";
    BookRecommendationService recommendation_;

    void showMainMenu() const;
    void showAdminMenu() const;
    void showUserMenu() const;

    void handleAdminSession();
    void handleUserSession();

    bool authenticateAdmin() const;
    Borrower* authenticateUser() const;

    void processAdminChoice(int choice, bool& keepRunning);
    void processUserChoice(Borrower* user, int choice, bool& keepRunning);

    void initializeFallbackData();
    bool saveLibrary();
    bool loadLibrary();
    void promptAndAddBook();
    void promptAndRemoveBook();
    void promptAndAddUser();
    void promptAndRemoveUser();
    void handleBorrowFlow(Borrower* user);
    void handleReturnFlow(Borrower* user);

#ifdef USE_MYSQL
    bool promptDatabaseConfig();
    bool withDbConnection(const std::function<void(db::DBManager&)>& fn) const;
    bool loadLibraryFromDatabase();
    bool persistLibraryToDatabase();
    bool syncBookToDatabase(int bookId);
#else
    bool promptDatabaseConfig() { return false; }
    bool loadLibraryFromDatabase() { return false; }
    bool persistLibraryToDatabase() { return false; }
    bool syncBookToDatabase(int) { return false; }
#endif
    bool loadFromFiles();
    bool saveToFiles() const;
};

} // namespace cli
