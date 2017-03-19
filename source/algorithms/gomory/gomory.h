#pragma once

#include <memory>
#include <common/document.h>

extern "C" {
	#include <gurobi_c.h>
}

class Gomory {
public:
	Gomory(const rapidjson::Value& root);
	~Gomory(void);
	void Run(void);

private:
	int num_vars, num_constrs, num_int_vars, status, grb_error;
	double epsilon;
	GRBenv* env;
	GRBmodel* model;
	GRBsvec* Binv_tmp;
	GRBsvec* svec_tmp;
};
