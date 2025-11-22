#include "FileManager.h"
#include "Library.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

namespace {

std::string escapeField(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '\t') out += "\\t";
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    return out;
}

std::string unescapeField(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '\\' && i + 1 < s.size()) {
            char next = s[++i];
            if (next == 't') out += '\t';
            else if (next == 'n') out += '\n';
            else out += next;
        } else {
            out += c;
        }
    }
    return out;
}

bool tryParseInt(const std::string& text, int& value) {
    try {
        size_t idx = 0;
        value = std::stoi(text, &idx);
        return idx == text.size();
    } catch (...) {
        return false;
    }
}

std::vector<std::string> splitTsv(const std::string& line) {
    std::vector<std::string> parts;
    std::string current;
    bool escape = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (!escape && c == '\t') {
            parts.push_back(current);
            current.clear();
            continue;
        }
        if (!escape && c == '\\') {
            escape = true;
            continue;
        }
        if (escape) {
            if (c == 't') current.push_back('\t');
            else if (c == 'n') current.push_back('\n');
            else current.push_back(c);
            escape = false;
        } else {
            current.push_back(c);
        }
    }
    parts.push_back(current);
    return parts;
}

bool ensureParentDirectory(const std::string& filename) {
    const std::filesystem::path path(filename);
    const auto parent = path.parent_path();
    if (parent.empty()) {
        return true;
    }
    std::error_code ec;
    std::filesystem::create_directories(parent, ec);
    if (ec) {
        std::cerr << "无法创建目录 " << parent << ": " << ec.message() << std::endl;
        return false;
    }
    return true;
}

template <typename Writer>
bool writeFileSafely(const std::string& filename, Writer&& writer, const char* description) {
    if (!ensureParentDirectory(filename)) {
        return false;
    }

    const std::filesystem::path finalPath(filename);
    std::filesystem::path tempPath = finalPath;
    tempPath += ".tmp";

    std::ofstream stream(tempPath, std::ios::trunc);
    if (!stream.is_open()) {
        std::cerr << "无法打开临时文件用于写入 " << description << ": " << tempPath << std::endl;
        return false;
    }

    writer(stream);

    if (!stream.good()) {
        std::cerr << "写入 " << description << " 时发生错误: " << filename << std::endl;
        stream.close();
        std::error_code ec;
        std::filesystem::remove(tempPath, ec);
        return false;
    }
    stream.close();

    std::error_code ec;
    std::filesystem::remove(finalPath, ec); // best-effort
    ec.clear();
    std::filesystem::rename(tempPath, finalPath, ec);
    if (ec) {
        std::cerr << "无法将临时文件替换为 " << description << ": " << ec.message() << std::endl;
        std::filesystem::remove(tempPath, ec);
        return false;
    }

    return true;
}

Borrower* createBorrowerFromParts(const std::vector<std::string>& parts) {
    if (parts.size() < 6) return nullptr;
    const std::string type = parts[0];
    const std::string& id = parts[1];
    const std::string& name = parts[2];
    const std::string& dept = parts[3];
    int limit = 0;
    if (!tryParseInt(parts[4], limit)) return nullptr;
    const std::string& extra = parts[5];

    if (type == "student" || type == "学生") {
        return new Student(id, name, dept, extra, limit);
    }
    if (type == "teacher" || type == "教师") {
        return new Teacher(id, name, dept, extra, limit);
    }
    return nullptr;
}

} // namespace

bool FileManager::saveBooksToFile(const std::vector<Book>& books, const std::string& filename) {
    bool succeeded = writeFileSafely(
        filename,
        [&books](std::ofstream& stream) {
            for (const auto& book : books) {
                stream << book.getBookId() << '\t'
                       << escapeField(book.getTitle()) << '\t'
                       << escapeField(book.getAuthor()) << '\t'
                       << escapeField(book.getIsbn()) << '\t'
                       << escapeField(book.getCategory()) << '\t'
                       << book.getTotalCopies() << '\t'
                       << book.getAvailableCopies() << '\n';
            }
        },
        "图书数据");
    if (succeeded) {
        std::cout << "Saved books to " << filename << std::endl;
    }
    return succeeded;
}

bool FileManager::loadBooksFromFile(std::vector<Book>& books, const std::string& filename) {
    std::ifstream stream(filename);
    if (!stream.is_open()) {
        std::cerr << "Cannot open " << filename << std::endl;
        return false;
    }
    books.clear();
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        auto parts = splitTsv(line);
        if (parts.size() < 7) {
            std::cerr << "跳过非法图书记录: " << line << std::endl;
            continue;
        }
        int id = 0, total = 0, available = 0;
        if (!tryParseInt(parts[0], id) || !tryParseInt(parts[5], total) || !tryParseInt(parts[6], available)) {
            std::cerr << "解析图书数字字段失败，记录已跳过: " << line << std::endl;
            continue;
        }
        Book book(id, unescapeField(parts[1]), unescapeField(parts[2]),
                  unescapeField(parts[3]), unescapeField(parts[4]), total);
        int borrowed = total - available;
        for (int i = 0; i < borrowed; ++i) book.borrowBook();
        books.push_back(book);
    }
    std::cout << "Loaded books from " << filename << std::endl;
    return true;
}

bool FileManager::saveBorrowersToFile(const std::vector<Borrower*>& borrowers, const std::string& filename) {
    bool succeeded = writeFileSafely(
        filename,
        [&borrowers](std::ofstream& stream) {
            for (auto borrower : borrowers) {
                if (!borrower) continue;
                std::string extra;
                if (borrower->getType() == "学生") {
                    if (const auto* stu = dynamic_cast<const Student*>(borrower)) {
                        extra = stu->getMajor();
                    }
                } else if (borrower->getType() == "教师") {
                    if (const auto* teacher = dynamic_cast<const Teacher*>(borrower)) {
                        extra = teacher->getTitle();
                    }
                }
                stream << escapeField(borrower->getType()) << '\t'
                       << escapeField(borrower->getId()) << '\t'
                       << escapeField(borrower->getName()) << '\t'
                       << escapeField(borrower->getDepartment()) << '\t'
                       << borrower->getMaxBorrowLimit() << '\t'
                       << escapeField(extra) << '\n';
            }
        },
        "用户数据");
    if (succeeded) {
        std::cout << "Saved users to " << filename << std::endl;
    }
    return succeeded;
}

bool FileManager::loadBorrowersFromFile(std::vector<Borrower*>& borrowers, const std::string& filename) {
    std::ifstream stream(filename);
    if (!stream.is_open()) {
        std::cerr << "Cannot open " << filename << std::endl;
        return false;
    }
    for (auto borrower : borrowers) delete borrower;
    borrowers.clear();

    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        auto parts = splitTsv(line);
        for (auto& part : parts) part = unescapeField(part);
        if (Borrower* borrower = createBorrowerFromParts(parts)) {
            borrowers.push_back(borrower);
        } else {
            std::cerr << "跳过非法用户记录: " << line << std::endl;
        }
    }
    std::cout << "Loaded users from " << filename << std::endl;
    return true;
}

Borrower* FileManager::loginUser(const std::string& filename, const std::string& id, const std::string& password) {
    std::string expected = id.substr(std::max(0, static_cast<int>(id.length()) - 6));
    if (password != expected) return nullptr;

    std::vector<Borrower*> users;
    if (!loadBorrowersFromFile(users, filename)) return nullptr;

    Borrower* found = nullptr;
    for (auto user : users) {
        if (user && user->getId() == id) {
            found = user;
            break;
        }
    }
    for (auto user : users) {
        if (user != nullptr && user != found) delete user;
    }
    users.clear();
    return found;
}

bool FileManager::saveLibrarySnapshot(const Library& library,
                                      const std::string& booksFile,
                                      const std::string& usersFile) {
    bool booksOk = saveBooksToFile(library.getBooks(), booksFile);
    bool usersOk = saveBorrowersToFile(library.getBorrowers(), usersFile);
    if (!booksOk || !usersOk) {
        std::cerr << "保存图书馆快照失败: " << (booksOk ? "" : "图书写入失败 ")
                  << (usersOk ? "" : "用户写入失败") << std::endl;
    } else {
        std::cout << "Library snapshot saved to " << booksFile << " / " << usersFile << std::endl;
    }
    return booksOk && usersOk;
}

bool FileManager::loadLibrarySnapshot(Library& library,
                                      const std::string& booksFile,
                                      const std::string& usersFile) {
    std::vector<Book> loadedBooks;
    std::vector<Borrower*> loadedBorrowers;

    bool booksOk = loadBooksFromFile(loadedBooks, booksFile);
    bool usersOk = loadBorrowersFromFile(loadedBorrowers, usersFile);

    if (!booksOk || !usersOk) {
        for (auto borrower : loadedBorrowers) delete borrower;
        loadedBorrowers.clear();
        std::cerr << "加载图书馆快照失败: " << (booksOk ? "" : "图书读取失败 ")
                  << (usersOk ? "" : "用户读取失败") << std::endl;
        return false;
    }

    library.setBooks(loadedBooks);
    library.setBorrowers(loadedBorrowers);
    std::cout << "Library snapshot loaded from " << booksFile << " / " << usersFile << std::endl;
    return true;
}
