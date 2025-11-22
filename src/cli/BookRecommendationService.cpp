#include "src/cli/BookRecommendationService.h"

#include <algorithm>
#include <iostream>

namespace cli {

void BookRecommendationService::recordBorrow(int bookId, const std::string& category) {
    borrowFrequency_[bookId] += 1;
    categoryBookMap_[category].push_back(bookId);
    std::cout << "推荐引擎已更新图书 " << bookId << " 的借阅统计" << std::endl;
}

std::vector<int> BookRecommendationService::popularBooks(std::size_t topN) const {
    std::vector<std::pair<int, int>> freq;
    freq.reserve(borrowFrequency_.size());
    for (const auto& entry : borrowFrequency_) {
        freq.emplace_back(entry.second, entry.first);
    }
    std::sort(freq.rbegin(), freq.rend());

    std::vector<int> result;
    topN = std::min(topN, freq.size());
    result.reserve(topN);
    for (std::size_t i = 0; i < topN; ++i) {
        result.push_back(freq[i].second);
    }
    return result;
}

void BookRecommendationService::printStatistics() const {
    if (borrowFrequency_.empty()) {
        std::cout << "暂无借阅统计数据。" << std::endl;
        return;
    }
    std::cout << "\n图书推荐统计 ===" << std::endl;
    std::cout << "借阅频率统计:" << std::endl;
    for (const auto& entry : borrowFrequency_) {
        std::cout << "图书ID " << entry.first << ": " << entry.second << " 次" << std::endl;
    }
    std::cout << "\n分类统计:" << std::endl;
    for (const auto& entry : categoryBookMap_) {
        std::cout << entry.first << " 分类: " << entry.second.size() << " 次借阅" << std::endl;
    }
}

} // namespace cli
