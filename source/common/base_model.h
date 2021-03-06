#pragma once

#include <memory>
#include "document.h"

#ifndef MAX_CUTS
#define MAX_CUTS 2500
#endif

extern "C" {
	#include <gurobi_c.h>
}

class BaseModel {
public:
	BaseModel(const rapidjson::Value& root, std::string model_path_,
	          std::string solution_path_);
	~BaseModel(void);
	virtual void Run(void);

protected:
	double optimal_objective;
	int grb_error, grb_status;
	int num_vars, num_constrs, num_int_vars, basis_size;
	std::string model_path, solution_path;
	
	GRBenv* env;
	GRBmodel* model;
};
