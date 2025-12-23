#include "Exchange.h"
#include "Broker.h"

#include <thread>
#include <chrono>
#include <iostream>

void Exchange::registerBroker(Broker& b) {
    std::lock_guard<std::mutex> lk(brokersMutex);
    brokers[b.id()] = &b;
}

size_t Exchange::genOrderId() {
    return nextId.fetch_add(1);
}

void Exchange::submitOrder(Order order) {
    if (order.id == 0)
        order.id = genOrderId();
    book.addOrder(order);
}

bool Exchange::bestBidPrice(double& out) { return book.bestBidPrice(out); }
bool Exchange::bestAskPrice(double& out) { return book.bestAskPrice(out); }

double Exchange::fairPriceEstimate() {
    std::lock_guard<std::mutex> lk(tradesMutex);
    if (allTrades.empty()) return -1.0;

    double sum = 0;
    int qty = 0;
    for (auto& t : allTrades) {
        sum += t.price * t.quantity;
        qty += t.quantity;
    }
    return qty ? sum / qty : -1.0;
}

void Exchange::setFee(double fee, int everyTicks) {
    feePerCycle = fee;
    feeEveryTicks = everyTicks;
}

void Exchange::stop() {
    running = false;
}

void Exchange::runLoop() {
    running = true;
    int t = 0;

    while (running.load()) {
        while (true) {
        	Trade tr;
        	if (!book.tryMatchOne(t, tr)) break;

        	// отменяем self-trade
        	if (tr.buyerId == tr.sellerId) {
        		// std::cerr << "buyerId = sellerId" << std::endl;
        		continue;
        	}

	        {
        		std::lock_guard<std::mutex> lk(tradesMutex);
        		allTrades.push_back(tr);
	        }

        	std::cout << "[t=" << t << "] TRADE: buyer=" << tr.buyerId
										<< " seller=" << tr.sellerId
										<< " qty=" << tr.quantity
										<< " price=" << tr.price
										<< "\n";

        	Broker* buyer = nullptr;
        	Broker* seller = nullptr;
	        {
        		std::lock_guard<std::mutex> lk(brokersMutex);
        		buyer  = brokers[tr.buyerId];
        		seller = brokers[tr.sellerId];
	        }

        	if (buyer)  buyer->applyTradeAsBuyer(tr.price, tr.quantity);
        	if (seller) seller->applyTradeAsSeller(tr.price, tr.quantity);
        }
    		// комиссия
        if (feeEveryTicks > 0 && t % feeEveryTicks == 0) {
            std::lock_guard<std::mutex> lk(brokersMutex);
            for (auto& [id, br] : brokers)
                br->applyFee(feePerCycle);
        }

        ++t;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
