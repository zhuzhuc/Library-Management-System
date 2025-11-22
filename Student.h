#ifndef STUDENT_H
#define STUDENT_H

#include "Borrower.h"

// Add missing virtual method declarations with override keyword
class Student : public Borrower {
private:
    std::string major; 

public:
    Student() : Borrower(), major("") {}
    Student(const std::string& id, const std::string& name, const std::string& dept, 
           const std::string& major, int maxLimit = 5)
        : Borrower(id, name, dept, maxLimit), major(major) {}
    
    // 虚函数
    std::string getType() const override { return "学生"; }
    void displayStudentInfo() const;
    void displayInfo() const override;

    // 特定方法
    bool searchBookInLibrary(Library& library, const std::string& title);
    std::vector<Book*> browseBooksByCategory(Library& library, const std::string& category);
    
    // 获取器
    std::string getMajor() const { return major; }
    
    // 设置器
    void setMajor(const std::string& newMajor) { major = newMajor; }
};

#endif // STUDENT_H