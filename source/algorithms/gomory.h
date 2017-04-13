#pragma once

#include <set>
#include <Eigen/Dense>
#include <common/base_model.h>

class Gomory : public BaseModel {
public:
	Gomory(const rapidjson::Value& root, std::string model_path_,
	       std::string solution_path_);
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
	void Optimize(void);

	// Define the different ways to add cuts.
	int AddPureCut(int cut_var_index);
	int AddMixedCut(int cur_var_index);
	int AddPureRounds(void);
	int AddMixedRounds(void);

	// Define ways to purge cuts.
	int PurgeCuts(void);

	// Methods to obtain fractional variable indices.
	int GetRandomIndex(void);
	int GetLexIndex(void);

	// User-defined parameters.
	double away_epsilon;
	double purge_epsilon;
	bool use_lex;
	bool use_rounds;
	bool use_fgmi;
	bool use_purging;
	int max_cuts;

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
	double old_objective_value, objective_value;

	// Declare required matrices and vectors.
	Eigen::MatrixXd B;
	Eigen::MatrixXd B_inv;
	Eigen::VectorXd r;
	Eigen::VectorXd c_beta;
	Eigen::VectorXd a_beta_r;
};
