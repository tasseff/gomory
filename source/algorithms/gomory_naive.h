#pragma once

#include <set>
#include <Eigen/Dense>
#include <common/base_model.h>

class GomoryNaive : public BaseModel {
public:
	GomoryNaive(const rapidjson::Value& root);
	~GomoryNaive(void);
	void Run(void);

protected:
	void ConvertVariables(void);
	void SetupModel(void);
	void UpdateBasisData(void);
	int UpdateVariableData(void);
	int AddPureCut(int cut_var_index);
	int AddMixedCut(int cur_var_index);

	// Declare other required variables.
	int basis_size, num_cuts;

	// Set of indices to keep track of currently fractional integer variables.
	std::set<int> frac_int_vars;

	// Declare pointers to arrays to store data about integer variables.
	int* int_var_ids;
	double* int_var_vals;

	// Declare pointers to arrays used to store single cut data.
	int* cut_coeff_ids;
	double* cut_coeff_vals;

	// Declare required matrices and vectors.
	Eigen::MatrixXd B;
	Eigen::MatrixXd B_inv;
	Eigen::VectorXd r;
	Eigen::VectorXd c_beta;
	Eigen::VectorXd a_beta_r;
};
