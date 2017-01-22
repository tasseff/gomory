#pragma once

#include <memory>
#include <common/document.h>
#include <gurobi_c++.h>

class Gomory {
public:
	Gomory(const rapidjson::Value& root);
	~Gomory(void);
	void Run(void);

private:
	GRBEnv* env;
	GRBModel* model;
};
