// 包含头文件
#include "src/cli/BookRecommendationService.h"

// 包含标准库
#include <algorithm> // 用于排序算法
#include <iostream>  // 用于输出

// 命名空间
namespace cli {

/**
 * @brief 记录图书借阅信息
 * @param bookId 图书ID
 * @param category 图书分类
 * @details 更新图书的借阅频率统计和分类映射信息
 */
void BookRecommendationService::recordBorrow(int bookId, const std::string& category) {
    // 增加图书的借阅频率计数
    borrowFrequency_[bookId] += 1;
    // 将图书ID添加到对应的分类中
    categoryBookMap_[category].push_back(bookId);
    // 输出操作日志
    std::cout << "推荐引擎已更新图书 " << bookId << " 的借阅统计" << std::endl;
}

/**
 * @brief 获取最受欢迎的图书列表
 * @param topN 需要返回的图书数量
 * @return 按借阅频率排序的图书ID列表
 * @details 根据借阅频率统计，返回借阅次数最多的N本图书ID
 */
std::vector<int> BookRecommendationService::popularBooks(std::size_t topN) const {
    // 创建借阅频率对（借阅次数，图书ID）的向量
    std::vector<std::pair<int, int>> freq;
    // 预分配内存以提高性能
    freq.reserve(borrowFrequency_.size());
    
    // 遍历借阅频率映射，将数据转换为可排序的向量
    for (const auto& entry : borrowFrequency_) {
        // 存储为（借阅次数，图书ID）的对，便于按借阅次数排序
        freq.emplace_back(entry.second, entry.first);
    }
    
    // 按借阅次数降序排序（从大到小）
    std::sort(freq.rbegin(), freq.rend());

    // 准备结果向量
    std::vector<int> result;
    // 确保不会超出实际图书数量
    topN = std::min(topN, freq.size());
    // 预分配结果向量的内存
    result.reserve(topN);
    
    // 提取前N个最受欢迎的图书ID
    for (std::size_t i = 0; i < topN; ++i) {
        result.push_back(freq[i].second); // freq[i].second 是图书ID
    }
    
    return result;
}

/**
 * @brief 打印借阅统计信息
 * @details 输出所有图书的借阅频率统计和各分类的借阅统计
 */
void BookRecommendationService::printStatistics() const {
    // 检查是否有借阅数据
    if (borrowFrequency_.empty()) {
        std::cout << "暂无借阅统计数据。" << std::endl;
        return;
    }
    
    // 输出标题
    std::cout << "\n图书推荐统计 ===" << std::endl;
    
    // 输出每本图书的借阅频率
    std::cout << "借阅频率统计:" << std::endl;
    for (const auto& entry : borrowFrequency_) {
        std::cout << "图书ID " << entry.first << ": " << entry.second << " 次" << std::endl;
    }
    
    // 输出每个分类的借阅统计
    std::cout << "\n分类统计:" << std::endl;
    for (const auto& entry : categoryBookMap_) {
        std::cout << entry.first << " 分类: " << entry.second.size() << " 次借阅" << std::endl;
    }
}

} // namespace cli