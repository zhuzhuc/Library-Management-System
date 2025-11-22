#ifndef TEACHER_H
#define TEACHER_H

#include "Borrower.h"

class Teacher : public Borrower {
private:
    std::string title; // 职称

public:
    Teacher() : Borrower(), title("") {}
    Teacher(const std::string& id, const std::string& name, const std::string& dept, 
           const std::string& title, int maxLimit = 10)
        : Borrower(id, name, dept, maxLimit), title(title) {}
    
    // 重写虚函数
    std::string getType() const override { return "教师"; }
    void displayInfo() const override;
    
    // 获取器
    std::string getTitle() const { return title; }
    
    // 设置器
    void setTitle(const std::string& newTitle) { title = newTitle; }
};

#endif // TEACHER_H