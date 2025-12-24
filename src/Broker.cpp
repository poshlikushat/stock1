#include "Broker.h"

Broker::Broker(int id, double cash, int inv, Exchange& ex)
    : _id(id), _cash(cash), _inventory(inv), _exchange(ex) {}

int Broker::id() const {
    return _id;
}

void Broker::applyTradeAsBuyer(double price, int qty) {
    std::lock_guard<std::mutex> lk(_mx);
    _cash -= price * qty;
    _inventory += qty;
}

void Broker::applyTradeAsSeller(double price, int qty) {
    std::lock_guard<std::mutex> lk(_mx);
    _cash += price * qty;
    _inventory -= qty;
}

void Broker::applyFee(double fee) {
    std::lock_guard<std::mutex> lk(_mx);
    _cash -= fee;
}

// ===== PlayerBroker =====
void PlayerBroker::step(int currentTime) {
    double fair = _exchange.fairPriceEstimate();
    if (fair <= 0) fair = 100.0;

    bool buy;
    {
        std::lock_guard<std::mutex> lk(_mx);
        if (hasRestingBuy && !hasRestingSell)
            buy = true;
        else if (hasRestingSell && !hasRestingBuy)
            buy = false;
        else
            buy = (uni(rng) < 0.5);
    }

    const double shift = uni(rng) * 0.14 - 0.07;
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

// ===== BigWinBroker =====
BigWinBroker::BigWinBroker(int id, double cash, int inv, Exchange& ex, double threshold)
    : Broker(id, cash, inv, ex), profitThreshold(threshold) {}

void BigWinBroker::step(int currentTime) {
    double fair = _exchange.fairPriceEstimate();
    if (fair <= 0) return;

    double bid{}, ask{};
    if (!_exchange.bestBidPrice(bid) || !_exchange.bestAskPrice(ask))
        return;

    if (ask < fair * (1.0 - profitThreshold)) {
        Order o{_id, OrderType::Limit, OrderSide::Buy, 0, 3, ask, currentTime};
        _exchange.submitOrder(o);
        return;
    }

    if (bid > fair * (1.0 + profitThreshold)) {
        Order o{_id, OrderType::Limit, OrderSide::Sell, 0, 3, bid, currentTime};
        _exchange.submitOrder(o);
    }
}

// ===== AnalystBroker =====
void AnalystBroker::step(int currentTime) {
    double fair = _exchange.fairPriceEstimate();
    if (fair <= 0) return;

    double bid{}, ask{};
    if (!_exchange.bestBidPrice(bid) || !_exchange.bestAskPrice(ask))
        return;

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
