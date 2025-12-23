#ifndef EXCHANGE_H
#define EXCHANGE_H

#include <atomic>
#include <mutex>
#include <vector>
#include <unordered_map>
#include "OrderBook.h"
#include "Order.h"

class Broker;

class Exchange final {
    OrderBook book;

    std::mutex tradesMutex;
    std::vector<Trade> allTrades;

    std::mutex brokersMutex;
    std::unordered_map<int, Broker*> brokers;

    std::atomic<bool> running{false};
    std::atomic<size_t> nextId{1};

    double feePerCycle = 1.0;
    int feeEveryTicks = 50;

public:
    void registerBroker(Broker& b);

    size_t genOrderId();
    void submitOrder(Order order);

    bool bestBidPrice(double& out);
    bool bestAskPrice(double& out);
    double fairPriceEstimate();

    void setFee(double fee, int everyTicks);
    void stop();
    void runLoop();
};

#endif
