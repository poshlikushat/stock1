#pragma once
#include "Broker.h"

class TestBroker final : public Broker {
public:
	using Broker::Broker;

	void step(int t) override {
	}
};
