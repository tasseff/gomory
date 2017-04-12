#pragma once

#include <set>
#include <Eigen/Dense>
#include <common/base_model.h>

class Gomory : public BaseModel {
public:
	Gomory(const rapidjson::Value& root);
	~Gomory(void);
	void Run(void);

protected:
	int Step(void);
	void ConvertVariables(void);
	void SetupModel(void);
	void UpdateBasisData(void);
	void PrintStep(void);
	int UpdateVariableData(void);
	void LexSimplex(void);

	// Define the different ways to add cuts.
	int AddPureCut(int cut_var_index);
	int AddMixedCut(int cur_var_index);
	int AddPureRounds(void);
	int AddMixedRounds(void);

	// Define ways to purge cuts.
	int PurgeCuts(void);

	// Methods to obtain fractional variable indices.
	int GetRandomIndex(void);
	int GetMostFractionalIndex(void);
	int GetLeastFractionalIndex(void);

	// Declare other required variables.
	int basis_size, num_cuts, original_num_constrs;

	// Set of indices to keep track of currently fractional integer variables.
	std::set<int> frac_int_vars;

	// Declare pointers to arrays to store data about integer variables.
	int* int_var_ids;
	double* int_var_vals;

	// Declare pointers to arrays used to store single cut data.
	int* cut_coeff_ids;
	double* cut_coeff_vals;

	// Stuff used for the lexicographic method.
	double* original_obj_coeffs; // Original objective coefficients.
	int* del_lex_constr_ids; // Indices of constraints to delete after lex step.

	// Declare required matrices and vectors.
	Eigen::MatrixXd B;
	Eigen::MatrixXd B_inv;
	Eigen::VectorXd r;
	Eigen::VectorXd c_beta;
	Eigen::VectorXd a_beta_r;

	// Define all the different epsilon types.
	double away_epsilon;
	bool use_rounds;
};
