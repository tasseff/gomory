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
  int get_random_var(const std::mt19937& rng, int size);
  GRBVar get_most_fractional(const std::unordered_set<unsigned int>& frac_var_ids,
    GRBVar* vars);
  GRBVar get_least_fractional(const std::unordered_set<unsigned int>& frac_var_ids,
    GRBVar* vars);

  double epsilon;
	GRBEnv* env;
	GRBModel* model;

};
