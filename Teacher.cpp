#include "Teacher.h"
#include <iostream>

void Teacher::displayInfo() const {
    Borrower::displayInfo(); // 调用基类方法
    std::cout << "职称: " << title << std::endl; // 添加职称信息
}