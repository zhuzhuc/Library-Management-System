#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
using namespace std;
// Forward declarations
class Book;
class Borrower;

namespace db {

    ///主要为CRUD
    class DBManager {
    public:
        DBManager();
        ~DBManager();

        bool connect(const string& host, unsigned port, const string& user,
                    const string& password, const string& dbname);
        void disconnect();

        bool createSchema();

        bool saveBooks(const vector<Book>& books);
        bool loadBooks(vector<Book>& outBooks);

        // single-object operations
        bool upsertBook(const Book& book);
        bool removeBook(int bookId);

        bool saveBorrowers(const vector<Borrower*>& borrowers);
        bool loadBorrowers(vector<Borrower*>& outBorrowers);

        bool upsertBorrower(Borrower* borrower);
        bool removeBorrower(const string& borrowerId);

        // Borrow records management
        bool createBorrowRecord(const string& borrowerId, int bookId, int borrowDays);
        bool returnBorrowRecord(int bookId, const string& borrowerId);
        bool getAllBorrowRecords(vector<map<string, string>>& outRecords);
        bool getBorrowRecordsByBorrower(const string& borrowerId, vector<map<string, string>>& outRecords);
        bool getBorrowRecordsByBook(int bookId, vector<map<string, string>>& outRecords);
        bool getActiveBorrowRecordsByBorrower(const string& borrowerId, vector<map<string, string>>& outRecords);
        bool getAllUsers(vector<map<string, string>>& outUsers);

        // User management (for login) 
        bool createUser(const string& username, const string& password, 
                       const string& userType, const string& borrowerId = "");
        bool authenticateUser(const string& username, const string& password, 
                             string& outUserType, string& outBorrowerId);
        bool updateUserPassword(const string& username, const string& newPassword);
        bool updateUserBorrowerId(const string& username, const string& borrowerId);
        bool userExists(const string& username);

        bool isConnected() const;

    private:
        struct Impl;
        unique_ptr<Impl> impl;
    };

} // namespace db
