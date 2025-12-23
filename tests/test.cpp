#include <gtest/gtest.h>
#include <thread>
#include "../src/Exchange.h"
#include "../src/Broker.h"
#include "../src/TestBroker.h"

class ExchangeTest : public ::testing::Test {
protected:
	Exchange ex;

	void SetUp() override {
		ex.setFee(0.0, 0);
	}
};

TEST_F(ExchangeTest, SimpleTradeHappens) {
	TestBroker buyer(1, 10000, 0, ex);
	TestBroker seller(2, 0, 100, ex);

	ex.registerBroker(buyer);
	ex.registerBroker(seller);

	Order buy;
	buy.brokerId = 1;
	buy.side = OrderSide::Buy;
	buy.type = OrderType::Limit;
	buy.price = 100;
	buy.quantity = 5;

	Order sell;
	sell.brokerId = 2;
	sell.side = OrderSide::Sell;
	sell.type = OrderType::Limit;
	sell.price = 99;
	sell.quantity = 5;

	ex.submitOrder(buy);
	ex.submitOrder(sell);

	std::thread t([&]{ ex.runLoop(); });
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	ex.stop();
	t.join();

	EXPECT_GT(ex.fairPriceEstimate(), 0);
}

TEST_F(ExchangeTest, SelfTradeIsRejected) {
	TestBroker b(1, 10000, 100, ex);
	ex.registerBroker(b);

	Order buy{1, OrderType::Limit, OrderSide::Buy, 0, 5, 100, 0};
	Order sell{1, OrderType::Limit, OrderSide::Sell, 0, 5, 99, 0};

	ex.submitOrder(buy);
	ex.submitOrder(sell);

	std::thread t([&]{ ex.runLoop(); });
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	ex.stop();
	t.join();

	EXPECT_LT(ex.fairPriceEstimate(), 0);
}


TEST_F(ExchangeTest, AllFourBrokersParticipate) {
	TestBroker b1(1, 10000, 100, ex);
	TestBroker b2(2, 10000, 100, ex);
	TestBroker b3(3, 10000, 100, ex);
	BigWinBroker big(4, 10000, 100, ex, 0.01);

	ex.registerBroker(b1);
	ex.registerBroker(b2);
	ex.registerBroker(b3);
	ex.registerBroker(big);

	ex.submitOrder({1, OrderType::Limit, OrderSide::Sell, 0, 10, 101, 0});
	ex.submitOrder({2, OrderType::Limit, OrderSide::Buy,  0, 10, 99,  0});

	ex.submitOrder({3, OrderType::Limit, OrderSide::Sell, 0, 10, 95,  0});

	std::thread exThread([&]{ ex.runLoop(); });

	std::thread bigThread([&]{
			for (int i = 0; i < 10; ++i) {
					big.step(i);
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	ex.stop();
	exThread.join();
	bigThread.join();

	EXPECT_GT(ex.fairPriceEstimate(), 0);
}

TEST(ExchangeBasic, EmptyMarketHasNoPrices) {
	Exchange ex;

	double bid{}, ask{};
	EXPECT_FALSE(ex.bestBidPrice(bid));
	EXPECT_FALSE(ex.bestAskPrice(ask));
}

TEST(ExchangeSafety, SelfTradeIgnored) {
	Exchange ex;

	ex.submitOrder({1, OrderType::Limit, OrderSide::Buy,  0, 5, 100, 0});
	ex.submitOrder({1, OrderType::Limit, OrderSide::Sell, 0, 5,  99, 0});

	std::thread t([&]{ ex.runLoop(); });
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	ex.stop();
	t.join();

	EXPECT_LT(ex.fairPriceEstimate(), 0);
}

TEST(ExchangeFees, FeeApplied) {
	Exchange ex;
	ex.setFee(1.0, 1);

	PlayerBroker p(1, 100.0, 10, ex);
	ex.registerBroker(p);

	std::thread t([&]{ ex.runLoop(); });
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	ex.stop();
	t.join();

	SUCCEED();
}

TEST(ExchangeQuotes, BestBidAskUpdate) {
	Exchange ex;

	ex.submitOrder({1, OrderType::Limit, OrderSide::Buy, 0, 1, 101, 0});
	ex.submitOrder({2, OrderType::Limit, OrderSide::Sell,0, 1, 103, 0});

	double bid{}, ask{};
	EXPECT_TRUE(ex.bestBidPrice(bid));
	EXPECT_TRUE(ex.bestAskPrice(ask));

	EXPECT_EQ(bid, 101);
	EXPECT_EQ(ask, 103);
}

TEST(Integration, MultiBrokerMarketLives) {
	Exchange ex;

	PlayerBroker p1(1,10000,50,ex);
	PlayerBroker p2(2,10000,50,ex);
	AnalystBroker a(3,10000,50,ex);
	BigWinBroker b(4,20000,100,ex,0.03);

	ex.registerBroker(p1);
	ex.registerBroker(p2);
	ex.registerBroker(a);
	ex.registerBroker(b);

	std::atomic<bool> alive{true};

	std::thread exT([&]{ ex.runLoop(); });

	auto run = [&](Broker& br){
		int t=0;
		while(alive){
			br.step(t++);
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
	};

	std::thread t1(run,std::ref(p1));
	std::thread t2(run,std::ref(p2));
	std::thread t3(run,std::ref(a));
	std::thread t4(run,std::ref(b));

	std::this_thread::sleep_for(std::chrono::milliseconds(300));
	alive=false; ex.stop();

	t1.join(); t2.join(); t3.join(); t4.join(); exT.join();

	EXPECT_GT(ex.fairPriceEstimate(), 0);
}







