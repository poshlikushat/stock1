#include <thread>
#include <atomic>
#include <chrono>
#include "src/Exchange.h"
#include "src/Broker.h"

int main() {
	Exchange ex;
	ex.setFee(0.5, 50);

	auto player  = std::make_shared<PlayerBroker>(1,  8000,  50, ex);
	auto player2 = std::make_shared<PlayerBroker>(2, 12000, 50, ex);
	auto analyst = std::make_shared<AnalystBroker>(3, 10000, 50, ex);
	auto big     = std::make_shared<BigWinBroker>(4, 20000, 100, ex, 0.03);

	ex.registerBroker(player);
	ex.registerBroker(player2);
	ex.registerBroker(analyst);
	ex.registerBroker(big);



	std::atomic<bool> alive{true};

	std::thread exchangeThread([&] { ex.runLoop(); });

	auto brokerLoop = [&](const std::shared_ptr<Broker>& br){
		int t = 0;
		while (alive) {
			br->step(t++);
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
	};


	std::thread t1(brokerLoop, player2);
	std::thread t2(brokerLoop, player);
	std::thread t3(brokerLoop, analyst);
	std::thread t4(brokerLoop, big);


	std::this_thread::sleep_for(std::chrono::seconds(5));

	alive = false;
	ex.stop();

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	exchangeThread.join();
}
