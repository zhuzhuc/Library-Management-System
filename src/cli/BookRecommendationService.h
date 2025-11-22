#pragma once

#include <map>
#include <string>
#include <vector>

namespace cli {

class BookRecommendationService {
public:
    void recordBorrow(int bookId, const std::string& category);
    std::vector<int> popularBooks(std::size_t topN = 3) const;
    void printStatistics() const;

private:
    std::map<std::string, std::vector<int>> categoryBookMap_;
    std::map<int, int> borrowFrequency_;
};

} // namespace cli
