#pragma once

#include <common/base_model.h>

class Gurobi : public BaseModel {
public:
	Gurobi(const rapidjson::Value& root);
	void Run(void);
};
