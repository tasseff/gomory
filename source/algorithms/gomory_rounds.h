#pragma once

#include "gomory_naive.h"

class GomoryRounds : public GomoryNaive {
public:
	GomoryRounds(const rapidjson::Value& root);
	void Run(void);
	int Step(void);

protected:
	int AddPureRounds(void);
	int AddMixedRounds(void);
};
