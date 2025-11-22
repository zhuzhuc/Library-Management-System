#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <vector>

// vector容器内容
template <typename T>
void printVector(const std::vector<T>& vec, const std::string& label) {
    std::cout << label << ": ";
    for (const auto& item : vec) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

#endif // UTILS_H