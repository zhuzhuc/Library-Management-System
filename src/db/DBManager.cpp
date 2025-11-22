#include "src/db/DBManager.h"
#include "Book.h"
#include "Student.h"
#include "Teacher.h"
#include <iostream>

#ifdef USE_MYSQL
#if __has_include(<mysql/mysql.h>)
#include <mysql/mysql.h>
#elif __has_include(<mysql.h>)
#include <mysql.h>
#else
#error "MySQL headers not found. Please install mysql-client library."
#endif
#endif

struct db::DBManager::Impl {
#ifdef USE_MYSQL
    MYSQL* conn = nullptr;
#else
    int dummy = 0;
#endif
};

using namespace std;
db::DBManager::DBManager() : impl(make_unique<Impl>()) {}

db::DBManager::~DBManager() { disconnect(); }

bool db::DBManager::connect(const string& host, unsigned port, const string& user,
                            const string& password, const string& dbname) {
#ifdef USE_MYSQL
    impl->conn = mysql_init(nullptr);
    if (!impl->conn) return false;
    if (!mysql_real_connect(impl->conn, host.c_str(), user.c_str(), password.c_str(), nullptr, port, nullptr, 0)) {
        cerr << "MySQL connection error (" << host << ":" << port << "): " << mysql_error(impl->conn) << endl;
        mysql_close(impl->conn);
        impl->conn = nullptr;
        return false;
    }
    if (!dbname.empty()) {
        std::string createDb = "CREATE DATABASE IF NOT EXISTS `" + dbname + "` CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci";
        if (mysql_query(impl->conn, createDb.c_str()) != 0) {
            cerr << "Warning: failed to ensure database exists (" << dbname << "): " << mysql_error(impl->conn) << endl;
        }
        if (mysql_select_db(impl->conn, dbname.c_str()) != 0) {
            cerr << "Unable to select database " << dbname << ": " << mysql_error(impl->conn) << endl;
            mysql_close(impl->conn);
            impl->conn = nullptr;
            return false;
        }
    }
    mysql_set_character_set(impl->conn, "utf8mb4");
    return true;
#else
    cerr << "MySQL support not enabled. Rebuild with -DUSE_MYSQL and link mysqlclient." << endl;
    return false;
#endif
}

void db::DBManager::disconnect() {
#ifdef USE_MYSQL
    if (impl->conn) {
        mysql_close(impl->conn);
        impl->conn = nullptr;
    }
#endif
}

bool db::DBManager::createSchema() {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    
    // Create books table
    const char* sql_books = R"(
    CREATE TABLE IF NOT EXISTS books (
        id INT PRIMARY KEY,
        title TEXT,
        author TEXT,
        isbn VARCHAR(64),
        category VARCHAR(128),
        total INT,
        available INT
    ))";
    if (mysql_query(impl->conn, sql_books) != 0) {
        cerr << "createSchema (books) failed: " << mysql_error(impl->conn) << endl;
        return false;
    }
    
    // Create borrowers table
    const char* sql_borrowers = R"(
    CREATE TABLE IF NOT EXISTS borrowers (
        id VARCHAR(64) PRIMARY KEY,
        type VARCHAR(32),
        name TEXT,
        department VARCHAR(128),
        max_limit INT,
        extra TEXT
    ))";
    if (mysql_query(impl->conn, sql_borrowers) != 0) {
        cerr << "createSchema (borrowers) failed: " << mysql_error(impl->conn) << endl;
        return false;
    }
    
    // Create borrow_records table for tracking borrow history
    const char* sql_records = R"(
    CREATE TABLE IF NOT EXISTS borrow_records (
        id INT AUTO_INCREMENT PRIMARY KEY,
        borrower_id VARCHAR(64),
        book_id INT,
        borrow_date DATETIME DEFAULT CURRENT_TIMESTAMP,
        borrow_days INT DEFAULT 7,
        expected_return_date DATETIME NOT NULL,
        return_date DATETIME NULL,
        status VARCHAR(32) DEFAULT 'borrowed',
        FOREIGN KEY (borrower_id) REFERENCES borrowers(id) ON DELETE CASCADE,
        FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE,
        INDEX idx_borrower (borrower_id),
        INDEX idx_book (book_id),
        INDEX idx_borrow_date (borrow_date),
        INDEX idx_status (status)
    ))";
    if (mysql_query(impl->conn, sql_records) != 0) {
        cerr << "createSchema (borrow_records) failed: " << mysql_error(impl->conn) << endl;
        cerr << "Warning: borrow_records table creation failed, continuing..." << endl;
    }
    
    // Alter table to add new columns if they don't exist (for existing databases)
    const char* sql_alter = R"(
        ALTER TABLE borrow_records 
        ADD COLUMN IF NOT EXISTS borrow_days INT DEFAULT 7,
        ADD COLUMN IF NOT EXISTS expected_return_date DATETIME,
        ADD COLUMN IF NOT EXISTS status VARCHAR(32) DEFAULT 'borrowed',
        ADD INDEX IF NOT EXISTS idx_status (status)
    )";
    // Note: MySQL doesn't support IF NOT EXISTS for ALTER TABLE ADD COLUMN
    // So we'll catch errors and continue
    mysql_query(impl->conn, "ALTER TABLE borrow_records ADD COLUMN borrow_days INT DEFAULT 7");
    mysql_query(impl->conn, "ALTER TABLE borrow_records ADD COLUMN expected_return_date DATETIME");
    mysql_query(impl->conn, "ALTER TABLE borrow_records ADD COLUMN status VARCHAR(32) DEFAULT 'borrowed'");
    // Ignore errors for existing columns
    
    // Create users table for authentication
    const char* sql_users = R"(
    CREATE TABLE IF NOT EXISTS users (
        username VARCHAR(64) PRIMARY KEY,
        password VARCHAR(255) NOT NULL,
        user_type VARCHAR(32) NOT NULL,
        borrower_id VARCHAR(64),
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
        INDEX idx_user_type (user_type),
        INDEX idx_borrower_id (borrower_id)
    ))";
    if (mysql_query(impl->conn, sql_users) != 0) {
        cerr << "createSchema (users) failed: " << mysql_error(impl->conn) << endl;
        return false;
    }
    
    // Insert default admin and user accounts if they don't exist
    const char* sql_default_users = R"(
    INSERT IGNORE INTO users (username, password, user_type) VALUES 
    ('admin', 'admin123', 'admin'),
    ('user', 'user123', 'user')
    )";
    if (mysql_query(impl->conn, sql_default_users) != 0) {
        cerr << "Warning: Failed to insert default users: " << mysql_error(impl->conn) << endl;
    }
    
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::saveBooks(const vector<Book>& books) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    // Use prepared statement for REPLACE INTO books
    const char* stmt_sql = "REPLACE INTO books (id,title,author,isbn,category,total,available) VALUES (?,?,?,?,?,?,?)";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) {
        cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }

    for (const auto& b : books) {
        MYSQL_BIND bind[7]; memset(bind, 0, sizeof(bind));
        // id
        int id = b.getBookId();
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = (char*)&id;
        // title
        string title = b.getTitle();
        bind[1].buffer_type = MYSQL_TYPE_STRING; bind[1].buffer = (char*)title.c_str(); bind[1].buffer_length = title.size();
        // author
        string author = b.getAuthor();
        bind[2].buffer_type = MYSQL_TYPE_STRING; bind[2].buffer = (char*)author.c_str(); bind[2].buffer_length = author.size();
        // isbn
        string isbn = b.getIsbn();
        bind[3].buffer_type = MYSQL_TYPE_STRING; bind[3].buffer = (char*)isbn.c_str(); bind[3].buffer_length = isbn.size();
        // category
        string category = b.getCategory();
        bind[4].buffer_type = MYSQL_TYPE_STRING; bind[4].buffer = (char*)category.c_str(); bind[4].buffer_length = category.size();
        // total
        int total = b.getTotalCopies(); bind[5].buffer_type = MYSQL_TYPE_LONG; bind[5].buffer = (char*)&total;
        // available
        int available = b.getAvailableCopies(); bind[6].buffer_type = MYSQL_TYPE_LONG; bind[6].buffer = (char*)&available;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            cerr << "bind failed: " << mysql_stmt_error(stmt) << endl;
            mysql_stmt_close(stmt);
            return false;
        }
        if (mysql_stmt_execute(stmt) != 0) {
            cerr << "execute failed: " << mysql_stmt_error(stmt) << endl;
            mysql_stmt_close(stmt);
            return false;
        }
    }
    mysql_stmt_close(stmt);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::loadBooks(vector<Book>& outBooks) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    if (mysql_query(impl->conn, "SELECT id, title, author, isbn, category, total, available FROM books") != 0) {
        cerr << "loadBooks query failed: " << mysql_error(impl->conn) << endl;
        return false;
    }
    MYSQL_RES* res = mysql_store_result(impl->conn);
    if (!res) return false;
    MYSQL_ROW row;
    outBooks.clear();
    while ((row = mysql_fetch_row(res))) {
        int id = atoi(row[0]);
        string title = row[1] ? row[1] : "";
        string author = row[2] ? row[2] : "";
        string isbn = row[3] ? row[3] : "";
        string category = row[4] ? row[4] : "";
        int total = row[5] ? atoi(row[5]) : 0;
        int available = row[6] ? atoi(row[6]) : total;
        Book b(id, title, author, isbn, category, total);
        int borrowCount = total - available;
        for (int i=0;i<borrowCount;i++) b.borrowBook();
        outBooks.push_back(b);
    }
    mysql_free_result(res);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::saveBorrowers(const vector<Borrower*>& borrowers) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = "REPLACE INTO borrowers (id,type,name,department,max_limit,extra) VALUES (?,?,?,?,?,?)";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) { cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl; mysql_stmt_close(stmt); return false; }

    for (auto b : borrowers) {
        string type = b->getType();
        string extra;
        Student* s = dynamic_cast<Student*>(b);
        if (s) extra = s->getMajor();
        Teacher* t = dynamic_cast<Teacher*>(b);
        if (t) extra = t->getTitle();

        MYSQL_BIND bind[6]; memset(bind, 0, sizeof(bind));
        string id = b->getId(); bind[0].buffer_type = MYSQL_TYPE_STRING; bind[0].buffer = (char*)id.c_str(); bind[0].buffer_length = id.size();
        bind[1].buffer_type = MYSQL_TYPE_STRING; bind[1].buffer = (char*)type.c_str(); bind[1].buffer_length = type.size();
        string name = b->getName(); bind[2].buffer_type = MYSQL_TYPE_STRING; bind[2].buffer = (char*)name.c_str(); bind[2].buffer_length = name.size();
        string dept = b->getDepartment(); bind[3].buffer_type = MYSQL_TYPE_STRING; bind[3].buffer = (char*)dept.c_str(); bind[3].buffer_length = dept.size();
        int limit = b->getMaxBorrowLimit(); bind[4].buffer_type = MYSQL_TYPE_LONG; bind[4].buffer = (char*)&limit;
        bind[5].buffer_type = MYSQL_TYPE_STRING; bind[5].buffer = (char*)extra.c_str(); bind[5].buffer_length = extra.size();

        if (mysql_stmt_bind_param(stmt, bind) != 0) { cerr << "bind failed: " << mysql_stmt_error(stmt) << endl; mysql_stmt_close(stmt); return false; }
        if (mysql_stmt_execute(stmt) != 0) { cerr << "execute failed: " << mysql_stmt_error(stmt) << endl; mysql_stmt_close(stmt); return false; }
    }
    mysql_stmt_close(stmt);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::loadBorrowers(vector<Borrower*>& outBorrowers) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    if (mysql_query(impl->conn, "SELECT id,type,name,department,max_limit,extra FROM borrowers") != 0) {
        cerr << "loadBorrowers query failed: " << mysql_error(impl->conn) << endl;
        return false;
    }
    MYSQL_RES* res = mysql_store_result(impl->conn);
    if (!res) return false;
    MYSQL_ROW row;
    for (auto b : outBorrowers) delete b;
    outBorrowers.clear();
    while ((row = mysql_fetch_row(res))) {
        string id = row[0] ? row[0] : "";
        string type = row[1] ? row[1] : "";
        string name = row[2] ? row[2] : "";
        string dept = row[3] ? row[3] : "";
        int limit = row[4] ? atoi(row[4]) : 5;
        string extra = row[5] ? row[5] : "";
        Borrower* br = nullptr;
        if (type == "student" || type == "\u5b66\u751f") {
            br = new Student(id, name, dept, extra, limit);
        } else if (type == "teacher" || type == "\u6559\u5e08") {
            br = new Teacher(id, name, dept, extra, limit);
        } else {
            continue;
        }
        outBorrowers.push_back(br);
    }
    mysql_free_result(res);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::isConnected() const {
#ifdef USE_MYSQL
    return impl->conn != nullptr;
#else
    return false;
#endif
}

bool db::DBManager::createUser(const string& username, const string& password, 
                                const string& userType, const string& borrowerId) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = "INSERT INTO users (username, password, user_type, borrower_id) VALUES (?, ?, ?, ?)";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) {
        cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[4]; memset(bind, 0, sizeof(bind));
    string user = username; bind[0].buffer_type = MYSQL_TYPE_STRING; bind[0].buffer = (char*)user.c_str(); bind[0].buffer_length = user.size();
    string pwd = password; bind[1].buffer_type = MYSQL_TYPE_STRING; bind[1].buffer = (char*)pwd.c_str(); bind[1].buffer_length = pwd.size();
    string type = userType; bind[2].buffer_type = MYSQL_TYPE_STRING; bind[2].buffer = (char*)type.c_str(); bind[2].buffer_length = type.size();
    string bid = borrowerId; bind[3].buffer_type = MYSQL_TYPE_STRING; bind[3].buffer = (char*)bid.c_str(); bind[3].buffer_length = bid.size();
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        cerr << "bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    if (mysql_stmt_execute(stmt) != 0) {
        cerr << "execute failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    mysql_stmt_close(stmt);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::authenticateUser(const string& username, const string& password, 
                                     string& outUserType, string& outBorrowerId) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = "SELECT user_type, borrower_id FROM users WHERE username = ? AND password = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) {
        cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[2]; memset(bind, 0, sizeof(bind));
    string user = username; bind[0].buffer_type = MYSQL_TYPE_STRING; bind[0].buffer = (char*)user.c_str(); bind[0].buffer_length = user.size();
    string pwd = password; bind[1].buffer_type = MYSQL_TYPE_STRING; bind[1].buffer = (char*)pwd.c_str(); bind[1].buffer_length = pwd.size();
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        cerr << "bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    if (mysql_stmt_execute(stmt) != 0) {
        cerr << "execute failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND result_bind[2]; memset(result_bind, 0, sizeof(result_bind));
    char user_type_buf[32]; unsigned long user_type_len = 0;
    char borrower_id_buf[64]; unsigned long borrower_id_len = 0;
    
    result_bind[0].buffer_type = MYSQL_TYPE_STRING;
    result_bind[0].buffer = user_type_buf;
    result_bind[0].buffer_length = sizeof(user_type_buf);
    result_bind[0].length = &user_type_len;
    
    result_bind[1].buffer_type = MYSQL_TYPE_STRING;
    result_bind[1].buffer = borrower_id_buf;
    result_bind[1].buffer_length = sizeof(borrower_id_buf);
    result_bind[1].length = &borrower_id_len;
    
    if (mysql_stmt_bind_result(stmt, result_bind) != 0) {
        cerr << "bind_result failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    bool found = false;
    int fetch_result = mysql_stmt_fetch(stmt);
    if (fetch_result == 0 || fetch_result == MYSQL_DATA_TRUNCATED) {
        if (user_type_len > 0 && user_type_len < sizeof(user_type_buf)) {
            outUserType = string(user_type_buf, user_type_len);
        } else {
            mysql_stmt_close(stmt);
            return false;
        }
        
        if (borrower_id_len > 0 && borrower_id_len < sizeof(borrower_id_buf)) {
            outBorrowerId = string(borrower_id_buf, borrower_id_len);
        } else {
            outBorrowerId = ""; // NULL or empty borrower_id
        }
        found = true;
    }
    
    mysql_stmt_close(stmt);
    return found;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::updateUserPassword(const string& username, const string& newPassword) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = "UPDATE users SET password = ? WHERE username = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) {
        cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[2]; memset(bind, 0, sizeof(bind));
    string pwd = newPassword; bind[0].buffer_type = MYSQL_TYPE_STRING; bind[0].buffer = (char*)pwd.c_str(); bind[0].buffer_length = pwd.size();
    string user = username; bind[1].buffer_type = MYSQL_TYPE_STRING; bind[1].buffer = (char*)user.c_str(); bind[1].buffer_length = user.size();
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        cerr << "bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    if (mysql_stmt_execute(stmt) != 0) {
        cerr << "execute failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    mysql_stmt_close(stmt);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::updateUserBorrowerId(const string& username, const string& borrowerId) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = "UPDATE users SET borrower_id = ? WHERE username = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) {
        cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[2]; memset(bind, 0, sizeof(bind));
    string borrower = borrowerId; bind[0].buffer_type = MYSQL_TYPE_STRING; bind[0].buffer = (char*)borrower.c_str(); bind[0].buffer_length = borrower.size();
    string user = username; bind[1].buffer_type = MYSQL_TYPE_STRING; bind[1].buffer = (char*)user.c_str(); bind[1].buffer_length = user.size();
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        cerr << "bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    if (mysql_stmt_execute(stmt) != 0) {
        cerr << "execute failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    mysql_stmt_close(stmt);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::userExists(const string& username) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = "SELECT username FROM users WHERE username = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) {
        cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind; memset(&bind, 0, sizeof(bind));
    string user = username; bind.buffer_type = MYSQL_TYPE_STRING; bind.buffer = (char*)user.c_str(); bind.buffer_length = user.size();
    
    if (mysql_stmt_bind_param(stmt, &bind) != 0) {
        cerr << "bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    if (mysql_stmt_execute(stmt) != 0) {
        cerr << "execute failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    char buf[64];
    MYSQL_BIND result_bind; memset(&result_bind, 0, sizeof(result_bind));
    result_bind.buffer_type = MYSQL_TYPE_STRING;
    result_bind.buffer = buf;
    result_bind.buffer_length = sizeof(buf);
    
    if (mysql_stmt_bind_result(stmt, &result_bind) != 0) {
        cerr << "bind_result failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    bool exists = (mysql_stmt_fetch(stmt) == 0);
    mysql_stmt_close(stmt);
    return exists;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

// Borrow records management
bool db::DBManager::createBorrowRecord(const string& borrowerId, int bookId, int borrowDays) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = "INSERT INTO borrow_records (borrower_id, book_id, borrow_days, expected_return_date, status) VALUES (?, ?, ?, DATE_ADD(NOW(), INTERVAL ? DAY), 'borrowed')";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) {
        cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[4]; memset(bind, 0, sizeof(bind));
    string bid = borrowerId; bind[0].buffer_type = MYSQL_TYPE_STRING; bind[0].buffer = (char*)bid.c_str(); bind[0].buffer_length = bid.size();
    bind[1].buffer_type = MYSQL_TYPE_LONG; bind[1].buffer = (char*)&bookId;
    bind[2].buffer_type = MYSQL_TYPE_LONG; bind[2].buffer = (char*)&borrowDays;
    bind[3].buffer_type = MYSQL_TYPE_LONG; bind[3].buffer = (char*)&borrowDays;
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        cerr << "bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    if (mysql_stmt_execute(stmt) != 0) {
        cerr << "execute failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    mysql_stmt_close(stmt);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::returnBorrowRecord(int bookId, const string& borrowerId) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = "UPDATE borrow_records SET return_date = NOW(), status = 'returned' WHERE book_id = ? AND borrower_id = ? AND return_date IS NULL";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) {
        cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[2]; memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG; bind[0].buffer = (char*)&bookId;
    string bid = borrowerId; bind[1].buffer_type = MYSQL_TYPE_STRING; bind[1].buffer = (char*)bid.c_str(); bind[1].buffer_length = bid.size();
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        cerr << "bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    if (mysql_stmt_execute(stmt) != 0) {
        cerr << "execute failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    mysql_stmt_close(stmt);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::getActiveBorrowRecordsByBorrower(const string& borrowerId, vector<map<string, string>>& outRecords) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = R"(
        SELECT br.id, br.book_id, br.borrower_id, br.borrow_date, br.borrow_days, 
               br.expected_return_date, br.return_date, br.status,
               b.title, b.author, b.isbn, b.category,
               bor.name as borrower_name
        FROM borrow_records br
        JOIN books b ON br.book_id = b.id
        LEFT JOIN borrowers bor ON br.borrower_id = bor.id
        WHERE br.borrower_id = ? AND br.status = 'borrowed'
        ORDER BY br.borrow_date DESC
    )";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) {
        cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind; memset(&bind, 0, sizeof(bind));
    string bid = borrowerId; bind.buffer_type = MYSQL_TYPE_STRING; bind.buffer = (char*)bid.c_str(); bind.buffer_length = bid.size();
    if (mysql_stmt_bind_param(stmt, &bind) != 0) {
        cerr << "bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        cerr << "execute failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_RES* res = mysql_stmt_result_metadata(stmt);
    if (!res) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    int num_fields = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);
    
    MYSQL_BIND* result_bind = new MYSQL_BIND[num_fields];
    unsigned long* lengths = new unsigned long[num_fields];
    char** buffers = new char*[num_fields];
    
    memset(result_bind, 0, sizeof(MYSQL_BIND) * num_fields);
    for (int i = 0; i < num_fields; i++) {
        buffers[i] = new char[256];
        result_bind[i].buffer_type = MYSQL_TYPE_STRING;
        result_bind[i].buffer = buffers[i];
        result_bind[i].buffer_length = 256;
        result_bind[i].length = &lengths[i];
    }
    
    if (mysql_stmt_bind_result(stmt, result_bind) != 0) {
        cerr << "bind_result failed: " << mysql_stmt_error(stmt) << endl;
        for (int i = 0; i < num_fields; i++) delete[] buffers[i];
        delete[] buffers;
        delete[] result_bind;
        delete[] lengths;
        mysql_free_result(res);
        mysql_stmt_close(stmt);
        return false;
    }
    
    while (mysql_stmt_fetch(stmt) == 0) {
        map<string, string> record;
        for (int i = 0; i < num_fields; i++) {
            string field_name = fields[i].name;
            string value(buffers[i], lengths[i]);
            record[field_name] = value;
        }
        outRecords.push_back(record);
    }
    
    for (int i = 0; i < num_fields; i++) delete[] buffers[i];
    delete[] buffers;
    delete[] result_bind;
    delete[] lengths;
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::getBorrowRecordsByBorrower(const string& borrowerId, vector<map<string, string>>& outRecords) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = R"(
        SELECT br.id, br.book_id, br.borrower_id, br.borrow_date, br.borrow_days, 
               br.expected_return_date, br.return_date, br.status,
               b.title, b.author, b.isbn, b.category,
               bor.name as borrower_name
        FROM borrow_records br
        JOIN books b ON br.book_id = b.id
        LEFT JOIN borrowers bor ON br.borrower_id = bor.id
        WHERE br.borrower_id = ?
        ORDER BY br.borrow_date DESC
    )";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) {
        cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind; memset(&bind, 0, sizeof(bind));
    string bid = borrowerId; bind.buffer_type = MYSQL_TYPE_STRING; bind.buffer = (char*)bid.c_str(); bind.buffer_length = bid.size();
    if (mysql_stmt_bind_param(stmt, &bind) != 0) {
        cerr << "bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        cerr << "execute failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_RES* res = mysql_stmt_result_metadata(stmt);
    if (!res) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    int num_fields = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);
    
    MYSQL_BIND* result_bind = new MYSQL_BIND[num_fields];
    unsigned long* lengths = new unsigned long[num_fields];
    char** buffers = new char*[num_fields];
    
    memset(result_bind, 0, sizeof(MYSQL_BIND) * num_fields);
    for (int i = 0; i < num_fields; i++) {
        buffers[i] = new char[256];
        result_bind[i].buffer_type = MYSQL_TYPE_STRING;
        result_bind[i].buffer = buffers[i];
        result_bind[i].buffer_length = 256;
        result_bind[i].length = &lengths[i];
    }
    
    if (mysql_stmt_bind_result(stmt, result_bind) != 0) {
        cerr << "bind_result failed: " << mysql_stmt_error(stmt) << endl;
        for (int i = 0; i < num_fields; i++) delete[] buffers[i];
        delete[] buffers;
        delete[] result_bind;
        delete[] lengths;
        mysql_free_result(res);
        mysql_stmt_close(stmt);
        return false;
    }
    
    while (mysql_stmt_fetch(stmt) == 0) {
        map<string, string> record;
        for (int i = 0; i < num_fields; i++) {
            string field_name = fields[i].name;
            string value(buffers[i], lengths[i]);
            record[field_name] = value;
        }
        outRecords.push_back(record);
    }
    
    for (int i = 0; i < num_fields; i++) delete[] buffers[i];
    delete[] buffers;
    delete[] result_bind;
    delete[] lengths;
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::getBorrowRecordsByBook(int bookId, vector<map<string, string>>& outRecords) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = R"(
        SELECT br.id, br.book_id, br.borrower_id, br.borrow_date, br.borrow_days, 
               br.expected_return_date, br.return_date, br.status,
               b.title, b.author, b.isbn, b.category,
               bor.name as borrower_name, bor.department as borrower_dept
        FROM borrow_records br
        JOIN books b ON br.book_id = b.id
        LEFT JOIN borrowers bor ON br.borrower_id = bor.id
        WHERE br.book_id = ?
        ORDER BY br.borrow_date DESC
    )";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) {
        cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind; memset(&bind, 0, sizeof(bind));
    bind.buffer_type = MYSQL_TYPE_LONG; bind.buffer = (char*)&bookId;
    if (mysql_stmt_bind_param(stmt, &bind) != 0) {
        cerr << "bind failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        cerr << "execute failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_RES* res = mysql_stmt_result_metadata(stmt);
    if (!res) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    int num_fields = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);
    
    MYSQL_BIND* result_bind = new MYSQL_BIND[num_fields];
    unsigned long* lengths = new unsigned long[num_fields];
    char** buffers = new char*[num_fields];
    
    memset(result_bind, 0, sizeof(MYSQL_BIND) * num_fields);
    for (int i = 0; i < num_fields; i++) {
        buffers[i] = new char[256];
        result_bind[i].buffer_type = MYSQL_TYPE_STRING;
        result_bind[i].buffer = buffers[i];
        result_bind[i].buffer_length = 256;
        result_bind[i].length = &lengths[i];
    }
    
    if (mysql_stmt_bind_result(stmt, result_bind) != 0) {
        cerr << "bind_result failed: " << mysql_stmt_error(stmt) << endl;
        for (int i = 0; i < num_fields; i++) delete[] buffers[i];
        delete[] buffers;
        delete[] result_bind;
        delete[] lengths;
        mysql_free_result(res);
        mysql_stmt_close(stmt);
        return false;
    }
    
    while (mysql_stmt_fetch(stmt) == 0) {
        map<string, string> record;
        for (int i = 0; i < num_fields; i++) {
            string field_name = fields[i].name;
            string value(buffers[i], lengths[i]);
            record[field_name] = value;
        }
        outRecords.push_back(record);
    }
    
    for (int i = 0; i < num_fields; i++) delete[] buffers[i];
    delete[] buffers;
    delete[] result_bind;
    delete[] lengths;
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::getAllBorrowRecords(vector<map<string, string>>& outRecords) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = R"(
        SELECT br.id, br.book_id, br.borrower_id, br.borrow_date, br.borrow_days, 
               br.expected_return_date, br.return_date, br.status,
               b.title, b.author, b.isbn, b.category,
               bor.name as borrower_name, bor.department as borrower_dept
        FROM borrow_records br
        JOIN books b ON br.book_id = b.id
        LEFT JOIN borrowers bor ON br.borrower_id = bor.id
        ORDER BY br.borrow_date DESC
    )";
    if (mysql_query(impl->conn, stmt_sql) != 0) {
        cerr << "getAllBorrowRecords failed: " << mysql_error(impl->conn) << endl;
        return false;
    }
    
    MYSQL_RES* res = mysql_store_result(impl->conn);
    if (!res) {
        cerr << "mysql_store_result failed: " << mysql_error(impl->conn) << endl;
        return false;
    }
    
    int num_fields = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);
    MYSQL_ROW row;
    
    while ((row = mysql_fetch_row(res))) {
        map<string, string> record;
        unsigned long* lengths = mysql_fetch_lengths(res);
        for (int i = 0; i < num_fields; i++) {
            string field_name = fields[i].name;
            string value = row[i] ? string(row[i], lengths[i]) : "";
            record[field_name] = value;
        }
        outRecords.push_back(record);
    }
    
    mysql_free_result(res);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::getAllUsers(vector<map<string, string>>& outUsers) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = R"(
        SELECT u.username, u.user_type, u.borrower_id, u.created_at,
               bor.name as borrower_name, bor.type as borrower_type, 
               bor.department as borrower_dept
        FROM users u
        LEFT JOIN borrowers bor ON u.borrower_id = bor.id
        ORDER BY u.created_at DESC
    )";
    if (mysql_query(impl->conn, stmt_sql) != 0) {
        cerr << "getAllUsers failed: " << mysql_error(impl->conn) << endl;
        return false;
    }
    
    MYSQL_RES* res = mysql_store_result(impl->conn);
    if (!res) {
        cerr << "mysql_store_result failed: " << mysql_error(impl->conn) << endl;
        return false;
    }
    
    int num_fields = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);
    MYSQL_ROW row;
    
    while ((row = mysql_fetch_row(res))) {
        map<string, string> user;
        unsigned long* lengths = mysql_fetch_lengths(res);
        for (int i = 0; i < num_fields; i++) {
            string field_name = fields[i].name;
            string value = row[i] ? string(row[i], lengths[i]) : "";
            user[field_name] = value;
        }
        outUsers.push_back(user);
    }
    
    mysql_free_result(res);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::upsertBook(const Book& book) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = "REPLACE INTO books (id,title,author,isbn,category,total,available) VALUES (?,?,?,?,?,?,?)";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) {
        cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl; mysql_stmt_close(stmt); return false; }

    MYSQL_BIND bind[7]; memset(bind, 0, sizeof(bind));
    int id = book.getBookId(); bind[0].buffer_type = MYSQL_TYPE_LONG; bind[0].buffer = (char*)&id;
    string title = book.getTitle(); bind[1].buffer_type = MYSQL_TYPE_STRING; bind[1].buffer = (char*)title.c_str(); bind[1].buffer_length = title.size();
    string author = book.getAuthor(); bind[2].buffer_type = MYSQL_TYPE_STRING; bind[2].buffer = (char*)author.c_str(); bind[2].buffer_length = author.size();
    string isbn = book.getIsbn(); bind[3].buffer_type = MYSQL_TYPE_STRING; bind[3].buffer = (char*)isbn.c_str(); bind[3].buffer_length = isbn.size();
    string category = book.getCategory(); bind[4].buffer_type = MYSQL_TYPE_STRING; bind[4].buffer = (char*)category.c_str(); bind[4].buffer_length = category.size();
    int total = book.getTotalCopies(); bind[5].buffer_type = MYSQL_TYPE_LONG; bind[5].buffer = (char*)&total;
    int available = book.getAvailableCopies(); bind[6].buffer_type = MYSQL_TYPE_LONG; bind[6].buffer = (char*)&available;

    if (mysql_stmt_bind_param(stmt, bind) != 0) { cerr << "bind failed: " << mysql_stmt_error(stmt) << endl; mysql_stmt_close(stmt); return false; }
    if (mysql_stmt_execute(stmt) != 0) { cerr << "execute failed: " << mysql_stmt_error(stmt) << endl; mysql_stmt_close(stmt); return false; }
    mysql_stmt_close(stmt);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::removeBook(int bookId) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = "DELETE FROM books WHERE id = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) { cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl; mysql_stmt_close(stmt); return false; }
    MYSQL_BIND bind; memset(&bind, 0, sizeof(bind));
    int id = bookId; bind.buffer_type = MYSQL_TYPE_LONG; bind.buffer = (char*)&id;
    if (mysql_stmt_bind_param(stmt, &bind) != 0) { cerr << "bind failed: " << mysql_stmt_error(stmt) << endl; mysql_stmt_close(stmt); return false; }
    if (mysql_stmt_execute(stmt) != 0) { cerr << "execute failed: " << mysql_stmt_error(stmt) << endl; mysql_stmt_close(stmt); return false; }
    mysql_stmt_close(stmt);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::upsertBorrower(Borrower* borrower) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = "REPLACE INTO borrowers (id,type,name,department,max_limit,extra) VALUES (?,?,?,?,?,?)";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) { cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl; mysql_stmt_close(stmt); return false; }

    string type = borrower->getType();
    string extra;
    Student* s = dynamic_cast<Student*>(borrower);
    if (s) extra = s->getMajor();
    Teacher* t = dynamic_cast<Teacher*>(borrower);
    if (t) extra = t->getTitle();

    MYSQL_BIND bind[6]; memset(bind, 0, sizeof(bind));
    string id = borrower->getId(); bind[0].buffer_type = MYSQL_TYPE_STRING; bind[0].buffer = (char*)id.c_str(); bind[0].buffer_length = id.size();
    bind[1].buffer_type = MYSQL_TYPE_STRING; bind[1].buffer = (char*)type.c_str(); bind[1].buffer_length = type.size();
    string name = borrower->getName(); bind[2].buffer_type = MYSQL_TYPE_STRING; bind[2].buffer = (char*)name.c_str(); bind[2].buffer_length = name.size();
    string dept = borrower->getDepartment(); bind[3].buffer_type = MYSQL_TYPE_STRING; bind[3].buffer = (char*)dept.c_str(); bind[3].buffer_length = dept.size();
    int limit = borrower->getMaxBorrowLimit(); bind[4].buffer_type = MYSQL_TYPE_LONG; bind[4].buffer = (char*)&limit;
    bind[5].buffer_type = MYSQL_TYPE_STRING; bind[5].buffer = (char*)extra.c_str(); bind[5].buffer_length = extra.size();

    if (mysql_stmt_bind_param(stmt, bind) != 0) { cerr << "bind failed: " << mysql_stmt_error(stmt) << endl; mysql_stmt_close(stmt); return false; }
    if (mysql_stmt_execute(stmt) != 0) { cerr << "execute failed: " << mysql_stmt_error(stmt) << endl; mysql_stmt_close(stmt); return false; }
    mysql_stmt_close(stmt);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}

bool db::DBManager::removeBorrower(const string& borrowerId) {
#ifdef USE_MYSQL
    if (!impl->conn) return false;
    const char* stmt_sql = "DELETE FROM borrowers WHERE id = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(impl->conn);
    if (!stmt) { cerr << "mysql_stmt_init failed: " << mysql_error(impl->conn) << endl; return false; }
    if (mysql_stmt_prepare(stmt, stmt_sql, strlen(stmt_sql)) != 0) { cerr << "prepare failed: " << mysql_stmt_error(stmt) << endl; mysql_stmt_close(stmt); return false; }
    MYSQL_BIND bind; memset(&bind, 0, sizeof(bind));
    string id = borrowerId; bind.buffer_type = MYSQL_TYPE_STRING; bind.buffer = (char*)id.c_str(); bind.buffer_length = id.size();
    if (mysql_stmt_bind_param(stmt, &bind) != 0) { cerr << "bind failed: " << mysql_stmt_error(stmt) << endl; mysql_stmt_close(stmt); return false; }
    if (mysql_stmt_execute(stmt) != 0) { cerr << "execute failed: " << mysql_stmt_error(stmt) << endl; mysql_stmt_close(stmt); return false; }
    mysql_stmt_close(stmt);
    return true;
#else
    cerr << "MySQL support not enabled." << endl;
    return false;
#endif
}
