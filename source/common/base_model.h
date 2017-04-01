#pragma once

#include <memory>
#include "document.h"

extern "C" {
	#include <gurobi_c.h>
}

class BaseModel {
public:
	BaseModel(const rapidjson::Value& root);
	~BaseModel(void);
	virtual void Run(void);

protected:
	double optimal_objective;
	int grb_error, grb_status;
	int num_vars, num_constrs, num_int_vars, basis_size;
	std::string solution_path;
	
	GRBenv* env;
	GRBmodel* model;
};
