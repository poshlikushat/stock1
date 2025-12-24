#pragma once

#include <mutex>
#include <random>
#include "Exchange.h"

class Broker {
protected:
    int _id;
    double _cash;
    int _inventory;
    Exchange& _exchange;

    std::mutex _mx;

    bool hasRestingBuy  = false;
    bool hasRestingSell = false;

public:
    Broker(int id, double cash, int inv, Exchange& ex);
    virtual ~Broker() = default;

    [[nodiscard]] int id() const;

    void applyTradeAsBuyer(double price, int qty);
    void applyTradeAsSeller(double price, int qty);
    void applyFee(double fee);

    virtual void step(int currentTime) = 0;
};

class PlayerBroker final : public Broker {
    std::mt19937 rng;
    std::uniform_real_distribution<double> uni;

public:
    using Broker::Broker;
    void step(int currentTime) override;
};

class BigWinBroker final : public Broker {
    double profitThreshold;

public:
    BigWinBroker(int id, double cash, int inv, Exchange& ex, double threshold);
    void step(int currentTime) override;
};

class AnalystBroker final : public Broker {
public:
    using Broker::Broker;
    void step(int currentTime) override;
};
