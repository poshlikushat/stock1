#include <thread>
#include <atomic>
#include <chrono>
#include "src/Exchange.h"
#include "src/Broker.h"

int main() {
	Exchange ex;
	ex.setFee(0.5, 50);

	PlayerBroker  player(1, 8000,  50, ex);
	PlayerBroker  player2(2, 12000, 50, ex);
	AnalystBroker analyst(3, 10000, 50, ex);
	BigWinBroker  big(4, 20000, 100, ex, 0.03);

	ex.registerBroker(player);
	ex.registerBroker(player2);
	ex.registerBroker(analyst);
	ex.registerBroker(big);


	std::atomic<bool> alive{true};

	std::thread exchangeThread([&] { ex.runLoop(); });

	auto brokerLoop = [&](Broker& b) {
		int t = 0;
		while (alive.load()) {
			b.step(t++);
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		}
	};

	std::thread t1(brokerLoop, std::ref(player2));
	std::thread t2(brokerLoop, std::ref(player));
	std::thread t3(brokerLoop, std::ref(analyst));
	std::thread t4(brokerLoop, std::ref(big));

	std::this_thread::sleep_for(std::chrono::seconds(5));

	alive = false;
	ex.stop();

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	exchangeThread.join();
}
