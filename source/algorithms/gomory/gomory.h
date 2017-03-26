#pragma once

#include <memory>
#include <common/document.h>

extern "C" {
	#include <gurobi_c.h>
}

class Gomory {
public:
	Gomory(const rapidjson::Value& root);
	unsigned int AddPureCut(int cut_var_index);
	unsigned int AddMixedCut(int cut_var_index);
	~Gomory(void);
	void Run(void);

private:
	int num_vars, num_constrs, num_int_vars, status, grb_error;
	unsigned int basis_size;
	double epsilon;

	Eigen::MatrixXd B;
	Eigen::MatrixXd Binv;
	Eigen::VectorXd r;
	Eigen::VectorXd c_beta;
	Eigen::VectorXd a_beta_r;

	int* cind;
	double* cval;

	GRBenv* env;
	GRBmodel* model;
	//GRBsvec* Binv_tmp;
	//GRBsvec* svec_tmp;
};
