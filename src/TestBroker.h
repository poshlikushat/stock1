#pragma once
#include "Broker.h"

class TestBroker : public Broker {
public:
	using Broker::Broker;

	void step(int t) override {
	}
};
