#pragma once

#include <memory>
#include <common/document.h>

extern "C" {
	#include <gurobi_c.h>
}

typedef std::unordered_set<unsigned int> index_set;

class Gomory {
public:
	Gomory(const rapidjson::Value& root);
	unsigned int AddPureCut(int cut_var_index);
	unsigned int AddMixedCut(int cut_var_index);
	void LexicographicSimplex(void);
	~Gomory(void);
	void Run(void);

private:
	int get_random_var(const index_set& frac_var_ids, std::mt19937 &rng);
	int get_most_fractional(const index_set& frac_var_ids);
	int get_least_fractional(const index_set& frac_var_ids);
	int num_vars, num_constrs, num_int_vars, status, grb_error;
	unsigned int basis_size;
	double epsilon;

	Eigen::MatrixXd B;
	Eigen::MatrixXd Binv;
	Eigen::VectorXd r;
	Eigen::VectorXd c_beta;
	Eigen::VectorXd a_beta_r;

	int* cind;
	int* del_constr_ids;
	double* cval;
	double* original_objs;

	GRBenv* env;
	GRBmodel* model;
};
