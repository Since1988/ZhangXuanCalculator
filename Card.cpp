#include "Card.h"
#include <algorithm>
#include <cctype>

// Card类实现
Card::Card(std::string name, Suit suit, CardType type, int profit)
    : m_name(std::move(name)), m_suit(suit), m_type(type), m_profit(profit) {}

std::string Card::getName() const { return m_name; }
Suit Card::getSuit() const { return m_suit; }
CardType Card::getType() const { return m_type; }
int Card::getProfit() const { return m_profit; }
void Card::setProfit(int newProfit) { m_profit = newProfit; }

std::string Card::getSuitName() const {
    return CardLibrary::suitToString(m_suit);
}

std::string Card::getFullName() const {
    return m_name + "_" + getSuitName();
}

// CardLibrary单例实现
CardLibrary& CardLibrary::getInstance() {
    static CardLibrary instance;
    return instance;
}

void CardLibrary::initDefaultCards() {
    // 收益权重说明：数值越高，优先级越高，越适合触发同礼
    // T0 天花板收益
    const int PROFIT_AOE = 10;    // 南蛮入侵、万箭齐发
    const int PROFIT_DUEL = 8;     // 决斗
    const int PROFIT_WUZHONG = 7;  // 无中生有
    const int PROFIT_SHUNSHOU = 6; // 顺手牵羊
    // T1 常规收益
    const int PROFIT_SHA = 5;      // 杀
    const int PROFIT_GUOHE = 4;    // 过河拆桥
    // T2 凑数收益
    const int PROFIT_EQUIP = 2;    // 装备牌
    const int PROFIT_OTHER_BASIC = 1; // 闪、桃、酒（非酒杀）
    const int PROFIT_DELAYED = 0;  // 延时锦囊（无收益）

    // 全花色卡牌模板，批量初始化
    std::vector<Suit> allSuits = {Suit::HEART, Suit::DIAMOND, Suit::SPADE, Suit::CLUB};

    // 1. 基本牌
    for (auto suit : allSuits) {
        addCard(Card("杀", suit, CardType::BASIC, PROFIT_SHA));
        addCard(Card("闪", suit, CardType::BASIC, PROFIT_OTHER_BASIC));
        addCard(Card("桃", suit, CardType::BASIC, PROFIT_OTHER_BASIC));
        addCard(Card("酒", suit, CardType::BASIC, PROFIT_OTHER_BASIC));
    }

    // 2. 普通锦囊牌
    for (auto suit : allSuits) {
        addCard(Card("决斗", suit, CardType::NORMAL_TRICK, PROFIT_DUEL));
        addCard(Card("过河拆桥", suit, CardType::NORMAL_TRICK, PROFIT_GUOHE));
        addCard(Card("顺手牵羊", suit, CardType::NORMAL_TRICK, PROFIT_SHUNSHOU));
        addCard(Card("无中生有", suit, CardType::NORMAL_TRICK, PROFIT_WUZHONG));
        addCard(Card("南蛮入侵", suit, CardType::NORMAL_TRICK, PROFIT_AOE));
        addCard(Card("万箭齐发", suit, CardType::NORMAL_TRICK, PROFIT_AOE));
        addCard(Card("借刀杀人", suit, CardType::NORMAL_TRICK, 3));
        addCard(Card("无懈可击", suit, CardType::NORMAL_TRICK, 2));
        addCard(Card("铁索连环", suit, CardType::NORMAL_TRICK, 3));
        addCard(Card("火攻", suit, CardType::NORMAL_TRICK, 4));
    }

    // 3. 装备牌（武器、防具、坐骑）
    std::vector<std::string> equipNames = {
        "诸葛连弩", "青龙偃月刀", "丈八蛇矛", "贯石斧", "方天画戟", "麒麟弓",
        "仁王盾", "八卦阵", "藤甲", "白银狮子",
        "+1马", "-1马"
    };
    for (auto& name : equipNames) {
        for (auto suit : allSuits) {
            addCard(Card(name, suit, CardType::EQUIP, PROFIT_EQUIP));
        }
    }

    // 4. 延时锦囊牌
    for (auto suit : allSuits) {
        addCard(Card("乐不思蜀", suit, CardType::DELAYED_TRICK, PROFIT_DELAYED));
        addCard(Card("兵粮寸断", suit, CardType::DELAYED_TRICK, PROFIT_DELAYED));
        addCard(Card("闪电", suit, CardType::DELAYED_TRICK, PROFIT_DELAYED));
    }
}

bool CardLibrary::hasCard(const std::string& fullName) const {
    return m_cardMap.find(fullName) != m_cardMap.end();
}

const Card* CardLibrary::getCard(const std::string& fullName) const {
    auto it = m_cardMap.find(fullName);
    return it != m_cardMap.end() ? &(it->second) : nullptr;
}

const Card* CardLibrary::getCard(const std::string& name, Suit suit) const {
    return getCard(name + "_" + suitToString(suit));
}

std::vector<const Card*> CardLibrary::getAllCards() const {
    std::vector<const Card*> res;
    for (auto& pair : m_cardMap) {
        res.push_back(&pair.second);
    }
    return res;
}

bool CardLibrary::addCard(const Card& card) {
    std::string fullName = card.getFullName();
    if (hasCard(fullName)) return false;
    m_cardMap[fullName] = card;
    return true;
}

bool CardLibrary::modifyCardProfit(const std::string& fullName, int newProfit) {
    auto it = m_cardMap.find(fullName);
    if (it == m_cardMap.end()) return false;
    it->second.setProfit(newProfit);
    return true;
}

bool CardLibrary::removeCard(const std::string& fullName) {
    return m_cardMap.erase(fullName) > 0;
}

Suit CardLibrary::stringToSuit(const std::string& suitStr) {
    std::string lowerStr = suitStr;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    if (lowerStr == "红桃" || lowerStr == "heart") return Suit::HEART;
    if (lowerStr == "方块" || lowerStr == "diamond") return Suit::DIAMOND;
    if (lowerStr == "黑桃" || lowerStr == "spade") return Suit::SPADE;
    if (lowerStr == "梅花" || lowerStr == "club") return Suit::CLUB;
    return Suit::NONE;
}

std::string CardLibrary::suitToString(Suit suit) {
    switch (suit) {
        case Suit::HEART: return "红桃";
        case Suit::DIAMOND: return "方块";
        case Suit::SPADE: return "黑桃";
        case Suit::CLUB: return "梅花";
        default: return "无花色";
    }
}

// 工具函数实现
int countSuitNumber(const std::vector<Card>& handCards) {
    std::set<Suit> suitSet;
    for (auto& card : handCards) {
        if (card.getSuit() != Suit::NONE) {
            suitSet.insert(card.getSuit());
        }
    }
    return suitSet.size();
}