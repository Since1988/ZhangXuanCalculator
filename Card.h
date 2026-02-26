#ifndef CARD_H
#define CARD_H

#include <string>
#include <unordered_map>
#include <vector>
#include <set>

// 卡牌花色枚举
enum class Suit {
    HEART,    // 红桃
    DIAMOND,  // 方块
    SPADE,    // 黑桃
    CLUB,     // 梅花
    NONE      // 无花色
};

// 卡牌类型枚举
enum class CardType {
    BASIC,           // 基本牌
    NORMAL_TRICK,    // 普通锦囊牌
    EQUIP,           // 装备牌
    DELAYED_TRICK    // 延时锦囊牌（同礼无收益，仅收录）
};

// 卡牌类
class Card {
public:
    Card() = default;
    Card(std::string name, Suit suit, CardType type, int profit);

    // 获取卡牌属性
    std::string getName() const;
    Suit getSuit() const;
    CardType getType() const;
    int getProfit() const;
    std::string getSuitName() const;
    std::string getFullName() const; // 名称_花色，唯一标识

    // 修改卡牌收益权重（支持自定义）
    void setProfit(int newProfit);

private:
    std::string m_name;       // 卡牌名称
    Suit m_suit;               // 卡牌花色
    CardType m_type;           // 卡牌类型
    int m_profit;              // 收益权重（核心，决定同礼收益优先级）
};

// 卡牌库单例类（全局唯一，内置全卡牌，支持自定义扩展）
class CardLibrary {
public:
    // 单例获取
    static CardLibrary& getInstance();

    // 禁用拷贝
    CardLibrary(const CardLibrary&) = delete;
    CardLibrary& operator=(const CardLibrary&) = delete;

    // 初始化内置卡牌库（标准+军争+一将成名相关卡牌）
    void initDefaultCards();

    // 卡牌查询
    bool hasCard(const std::string& fullName) const;
    const Card* getCard(const std::string& fullName) const;
    const Card* getCard(const std::string& name, Suit suit) const;
    std::vector<const Card*> getAllCards() const;

    // 自定义卡牌操作
    bool addCard(const Card& card);
    bool modifyCardProfit(const std::string& fullName, int newProfit);
    bool removeCard(const std::string& fullName);

    // 工具方法：花色/名称转换
    static Suit stringToSuit(const std::string& suitStr);
    static std::string suitToString(Suit suit);

private:
    CardLibrary() { initDefaultCards(); } // 私有构造
    std::unordered_map<std::string, Card> m_cardMap; // key: 卡牌全称
};

// 工具函数：统计手牌的花色数量
int countSuitNumber(const std::vector<Card>& handCards);

#endif // CARD_H