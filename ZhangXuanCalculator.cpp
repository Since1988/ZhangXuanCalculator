#include "ZhangXuanCalculator.h"
#include <algorithm>

// 核心计算入口
CalculateResult ZhangXuanCalculator::calculateOptimalPlay(const std::vector<Card>& handCards) {
    // 重置最优结果
    m_bestResult = CalculateResult();
    if (handCards.empty()) {
        return m_bestResult;
    }

    // 初始化回溯变量
    std::vector<Card> currentHand = handCards;
    std::vector<Card> currentOrder;
    backtrack(currentHand, currentOrder, 0, 0);

    // 计算触发信息
    for (int i = 0; i < m_bestResult.optimalOrder.size(); i++) {
        // 模拟出牌过程，找到触发位置
        std::vector<Card> tempHand = handCards;
        int usedCount = 0;
        int triggerExtra = 0;
        bool isTrigger = false;

        for (int j = 0; j <= i; j++) {
            usedCount++;
            // 移除打出的牌
            auto it = std::find_if(tempHand.begin(), tempHand.end(),
                [&](const Card& c) { return c.getFullName() == m_bestResult.optimalOrder[j].getFullName(); });
            if (it != tempHand.end()) {
                tempHand.erase(it);
            }
            // 计算触发条件
            int suitCount = countSuitNumber(tempHand);
            if (suitCount == usedCount) {
                isTrigger = true;
                triggerExtra = suitCount;
                break;
            }
        }

        if (isTrigger) {
            m_bestResult.triggerIndex = i;
            m_bestResult.triggerExtraTimes = triggerExtra;
            m_bestResult.triggerCardName = m_bestResult.optimalOrder[i].getFullName();
            break;
        }
    }

    return m_bestResult;
}

// 回溯递归实现
void ZhangXuanCalculator::backtrack(std::vector<Card>& currentHand, std::vector<Card>& currentOrder,
                                     int currentProfit, int usedCount) {
    // 递归终止：手牌出完，更新最优结果
    if (currentHand.empty()) {
        if (currentProfit > m_bestResult.maxTotalProfit) {
            m_bestResult.maxTotalProfit = currentProfit;
            m_bestResult.optimalOrder = currentOrder;
        }
        return;
    }

    // 枚举所有可选的下一张出牌
    for (int i = 0; i < currentHand.size(); i++) {
        Card selectedCard = currentHand[i];
        // 1. 生成新的手牌（移除当前选中的牌）
        std::vector<Card> newHand = currentHand;
        newHand.erase(newHand.begin() + i);
        // 2. 计算核心参数
        int newUsedCount = usedCount + 1;
        int newSuitCount = countSuitNumber(newHand);
        // 3. 计算当前牌的收益（严格遵循官方同礼规则）
        int cardProfit = selectedCard.getProfit();
        if (newSuitCount == newUsedCount) {
            cardProfit *= (newSuitCount + 1); // 额外执行X次，总次数X+1
        }
        // 4. 回溯递归
        currentOrder.push_back(selectedCard);
        backtrack(newHand, currentOrder, currentProfit + cardProfit, newUsedCount);
        currentOrder.pop_back(); // 撤销选择
    }
}