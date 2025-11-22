#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>
#include <vector>
#include "Book.h"
#include "Borrower.h"
#include "Student.h"
#include "Teacher.h"

class Library;

class FileManager {
public:
    // 图书数据文件操作
    static bool saveBooksToFile(const std::vector<Book>& books, const std::string& filename);
    static bool loadBooksFromFile(std::vector<Book>& books, const std::string& filename);
    
    // 用户数据文件操作
    static bool saveBorrowersToFile(const std::vector<Borrower*>& borrowers, const std::string& filename);
    static bool loadBorrowersFromFile(std::vector<Borrower*>& borrowers, const std::string& filename);
    
    // 登录功能
    static Borrower* loginUser(const std::string& filename, const std::string& id, const std::string& password);

    // 统一快照
    static bool saveLibrarySnapshot(const Library& library,
                                    const std::string& booksFile,
                                    const std::string& usersFile);
    static bool loadLibrarySnapshot(Library& library,
                                    const std::string& booksFile,
                                    const std::string& usersFile);
};

#endif // FILEMANAGER_H