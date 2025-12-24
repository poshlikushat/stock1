#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <map>
#include <unordered_map>
#include <mutex>
#include "Order.h"

class OrderBook final {
    std::unordered_map<size_t, std::multimap<double, Order>::iterator> indexBid;
    std::unordered_map<size_t, std::multimap<double, Order>::iterator> indexAsk;

    std::multimap<double, Order, std::greater<>> bids;          // лучшая цена первая
    std::multimap<double, Order> asks;                          // лучшая (минимальная) цена первая

    std::mutex m;

public:
    void addOrder(const Order& order);

    bool tryMatchOne(int currentTime, Trade& out);

    bool bestBidPrice(double& out);

    bool bestAskPrice(double& out);
};

#endif // ORDERBOOK_H
