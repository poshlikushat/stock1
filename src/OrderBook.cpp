#include <mutex>
#include "OrderBook.h"

void OrderBook::addOrder(const Order& order) {
	std::lock_guard<std::mutex> lk(m);

	Order o = order;
	if (o.type == OrderType::Market) {
		if (o.side == OrderSide::Buy)  o.price = 1e100;
		else                           o.price = 0.0;
		o.type = OrderType::Limit;
	}

	if (o.quantity <= 0) return;

	if (o.side == OrderSide::Buy) {
		auto it = bids.emplace(o.price, o);
		indexBid[o.id] = it;
	} else {
		auto it = asks.emplace(o.price, o);
		indexAsk[o.id] = it;
	}
}

bool OrderBook::tryMatchOne(int currentTime, Trade& out) {
	std::lock_guard<std::mutex> lk(m);

	if (bids.empty() || asks.empty()) return false;

	auto itBid = bids.begin(); // max price
	auto itAsk = asks.begin(); // min price

	double bidPrice = itBid->first;
	double askPrice = itAsk->first;

	if (bidPrice < askPrice) return false;

	Order& buy  = itBid->second;
	Order& sell = itAsk->second;

	int qty = std::min(buy.quantity, sell.quantity);
	if (qty <= 0) return false;

	out.buyerId = buy.brokerId;
	out.sellerId = sell.brokerId;
	out.price = askPrice;
	out.quantity = qty;
	out.executedAt = currentTime;

	buy.quantity -= qty;
	sell.quantity -= qty;

	if (buy.quantity == 0) {
		indexBid.erase(buy.id);
		bids.erase(itBid);
	}
	if (sell.quantity == 0) {
		indexAsk.erase(sell.id);
		asks.erase(itAsk);
	}
	return true;
}

bool OrderBook::bestBidPrice(double& out) {
	{
		std::lock_guard<std::mutex> lk(m);
		if (bids.empty()) return false;
		out = bids.begin()->first;
		return true;
	}
}

bool OrderBook::bestAskPrice(double& out) {
	{
		std::lock_guard<std::mutex> lk(m);
		if (asks.empty()) return false;
		out = asks.begin()->first;
		return true;
	}
}