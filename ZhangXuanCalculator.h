#ifndef ZHANGXUANCALCULATOR_H
#define ZHANGXUANCALCULATOR_H

#include "Card.h"
#include <vector>

// 计算结果结构体
struct CalculateResult {
    std::vector<Card> optimalOrder;       // 最优出牌顺序
    int maxTotalProfit = 0;                // 最大总收益
    int triggerIndex = -1;                 // 同礼触发位置（从0开始，-1为未触发）
    int triggerExtraTimes = 0;              // 同礼额外执行次数
    std::string triggerCardName;           // 触发同礼的卡牌全称
};

// 张嫙同礼收益计算器类
class ZhangXuanCalculator {
public:
    ZhangXuanCalculator() = default;

    // 核心计算函数：输入手牌，返回最优结果
    CalculateResult calculateOptimalPlay(const std::vector<Card>& handCards);

private:
    // 回溯递归函数：枚举所有出牌顺序，求解最优解
    void backtrack(std::vector<Card>& currentHand, std::vector<Card>& currentOrder,
                   int currentProfit, int usedCount);

    // 全局最优结果存储
    CalculateResult m_bestResult;
};

#endif // ZHANGXUANCALCULATOR_H