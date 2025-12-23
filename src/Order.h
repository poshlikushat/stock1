#ifndef ORDER_H
#define ORDER_H

#include <cstddef>

enum class OrderType { Market, Limit };
enum class OrderSide { Buy, Sell };

struct Trade final {
    int buyerId{};
    int sellerId{};
    double price{};
    int quantity{};
    int executedAt{};
};

struct Order final {
    int brokerId{};
    OrderType type{OrderType::Limit};
    OrderSide side{OrderSide::Buy};
    std::size_t id{};
    int quantity{};
    double price{};
    int createdAt{};
};

#endif // ORDER_H
