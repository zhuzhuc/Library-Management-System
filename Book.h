#ifndef BOOK_FINAL_H
#define BOOK_FINAL_H

#include <string>
#include <iostream>

class Book {
private:
    int bookId;
    std::string title;
    std::string author;
    std::string isbn;
    std::string category;
    int totalCopies;
    int availableCopies;
    bool isAvailable;

public:
    Book();
    Book(int id, const std::string& title, const std::string& author, 
         const std::string& isbn, const std::string& category, int copies);
    
    // 基本访问方法
    int getBookId() const { return bookId; }
    std::string getTitle() const { return title; }
    std::string getAuthor() const { return author; }
    std::string getIsbn() const { return isbn; }
    std::string getCategory() const { return category; }
    int getTotalCopies() const { return totalCopies; }
    int getAvailableCopies() const { return availableCopies; }
    bool getIsAvailable() const { return isAvailable; }
    
    // 业务方法
    bool borrowBook();
    bool returnBook();
    void displayBookInfo() const;
    void displayDetailedInfo() const;
    
protected:
    void updateAvailability();
};

#endif
