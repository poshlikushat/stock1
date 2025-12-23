#ifndef BROKER_H
#define BROKER_H

#include <mutex>
#include <random>
#include <cmath>
#include <vector>
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
    Broker(int id, double cash, int inv, Exchange& ex)
        : _id(id), _cash(cash), _inventory(inv), _exchange(ex) {}

    virtual ~Broker() = default;

    [[nodiscard]] int id() const { return _id; }

    // биржа вызывает это при сделках/комиссии
    void applyTradeAsBuyer(double price, int qty) {
        std::lock_guard<std::mutex> lk(_mx);
        _cash -= price * qty;
        _inventory += qty;
    }
    void applyTradeAsSeller(double price, int qty) {
        std::lock_guard<std::mutex> lk(_mx);
        _cash += price * qty;
        _inventory -= qty;
    }
    void applyFee(double fee) {
        std::lock_guard<std::mutex> lk(_mx);
        _cash -= fee;
    }

    // брокерская логика
    virtual void step(int currentTime) = 0;
};

// PlayerBroker: много рандома около "средней"
class PlayerBroker final : public Broker {
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<double> uni{0.0, 1.0};
public:
    using Broker::Broker;

    void step(int currentTime) override {
        double fair = _exchange.fairPriceEstimate(); // средняя по сделкам
        if (fair <= 0) fair = 100.0;

        bool buy;
        {
            std::lock_guard<std::mutex> lk(_mx);

            if (hasRestingBuy && !hasRestingSell)
                buy = true;          // уже есть BUY → только BUY
            else if (hasRestingSell && !hasRestingBuy)
                buy = false;         // уже есть SELL → только SELL
            else
                buy = (uni(rng) < 0.5);
        }


        // цена вокруг fair (±7%)
        const double shift = (uni(rng) * 0.14 - 0.07);;
        const double price = fair * (1.0 + shift);

        const int qty = 3 + static_cast<int>(uni(rng) * 8);


        Order o;
        o.brokerId = _id;
        o.type = OrderType::Limit;
        o.side = buy ? OrderSide::Buy : OrderSide::Sell;
        o.quantity = qty;
        o.price = price;
        o.createdAt = currentTime;

        _exchange.submitOrder(o);
    }
};

// BigWinBroker: ждёт сильно выгодно
class BigWinBroker final : public Broker {
    double profitThreshold;
public:
    BigWinBroker(int id, double cash, int inv, Exchange& ex, double threshold)
        : Broker(id, cash, inv, ex), profitThreshold(threshold) {}

    void step(int currentTime) override {
        double fair = _exchange.fairPriceEstimate();
        if (fair <= 0) return;

        double bid{}, ask{};
        bool hasBid = _exchange.bestBidPrice(bid);
        bool hasAsk = _exchange.bestAskPrice(ask);
        if (!hasBid || !hasAsk) return;

        // Если можно купить сильно ниже средней — покупаем
        if (ask < fair * (1.0 - profitThreshold)) {
            Order o;
            o.brokerId = _id;
            o.type = OrderType::Limit;
            o.side = OrderSide::Buy;
            o.quantity = 3;
            o.price = ask;
            o.createdAt = currentTime;
            _exchange.submitOrder(o);
            return;
        }

        // Если можно продать сильно выше средней — продаём
        if (bid > fair * (1.0 + profitThreshold)) {
            Order o;
            o.brokerId = _id;
            o.type = OrderType::Limit;
            o.side = OrderSide::Sell;
            o.quantity = 3;
            o.price = bid;
            o.createdAt = currentTime;
            _exchange.submitOrder(o);
        }
    }
};

// AnalystBroker: если ниже средней — buy, выше — sell
class AnalystBroker final : public Broker {
public:
    using Broker::Broker;

    void step(int currentTime) override {
        double fair = _exchange.fairPriceEstimate();
        if (fair <= 0) return;

        double bid{}, ask{};
        bool hasBid = _exchange.bestBidPrice(bid);
        bool hasAsk = _exchange.bestAskPrice(ask);
        if (!hasBid || !hasAsk) return;

        double mid = 0.5 * (bid + ask);

        Order o;
        o.brokerId = _id;
        o.type = OrderType::Limit;
        o.quantity = 2;
        o.createdAt = currentTime;

        if (mid < fair) {
            o.side = OrderSide::Buy;
            o.price = ask;
        } else if (mid > fair) {
            o.side = OrderSide::Sell;
            o.price = bid;
        } else {
            return;
        }

        _exchange.submitOrder(o);
    }
};

#endif // BROKER_H
