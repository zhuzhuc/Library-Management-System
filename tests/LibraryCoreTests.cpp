#include "FileManager.h"
#include "Library.h"
#include "Student.h"

#include <cassert>
#include <filesystem>
#include <iostream>

int main() {
    Library library("Test Library", "Unit Test");
    library.addBook(Book(1, "C++ Primer", "Lippman", "ISBN-001", "CS", 3));
    library.addBook(Book(2, "Design Patterns", "GoF", "ISBN-002", "CS", 2));

    auto* user = new Student("2023001", "测试用户", "计算机学院", "软件工程", 3);
    library.addBorrower(user);
    assert(library.findBookById(1) != nullptr);

    bool borrowOk = user->borrowBookFromLibrary(library, 1);
    assert(borrowOk);
    Book* tracked = library.findBookById(1);
    assert(tracked != nullptr);
    assert(tracked->getAvailableCopies() == tracked->getTotalCopies() - 1);
    user->returnBookToLibrary(library, 1);
    assert(tracked->getAvailableCopies() == tracked->getTotalCopies());

    std::filesystem::path tempDir = std::filesystem::temp_directory_path();
    auto booksFile = tempDir / "library_books_test.tsv";
    auto usersFile = tempDir / "library_users_test.tsv";

    assert(FileManager::saveBooksToFile(library.getBooks(), booksFile.string()));
    assert(FileManager::saveBorrowersToFile(library.getBorrowers(), usersFile.string()));

    std::vector<Book> loadedBooks;
    std::vector<Borrower*> loadedUsers;
    assert(FileManager::loadBooksFromFile(loadedBooks, booksFile.string()));
    assert(FileManager::loadBorrowersFromFile(loadedUsers, usersFile.string()));
    assert(!loadedBooks.empty());
    assert(!loadedUsers.empty());

    for (auto borrower : loadedUsers) delete borrower;
    std::filesystem::remove(booksFile);
    std::filesystem::remove(usersFile);

    std::cout << "Library core tests passed." << std::endl;
    return 0;
}
