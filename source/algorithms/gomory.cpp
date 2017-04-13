#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>
#include "gomory.h"

Gomory::Gomory(const rapidjson::Value& root, std::string model_path_,
               std::string solution_path_) : BaseModel(root, model_path_,
                                                       solution_path_) {
	// Load required user-defined parameters.
	max_cuts = root["maxCuts"].GetInt();
	away_epsilon = root["awayEpsilon"].GetDouble();
	purge_epsilon = root["purgeEpsilon"].GetDouble();
	use_lex = root["useLexicographic"].GetBool();
	use_rounds = root["useRounds"].GetBool();
	use_fgmi = root["useMixedCut"].GetBool();
	use_purging = root["usePurging"].GetBool();

	// Set up the Gurobi environment variable.
	grb_error = GRBloadenv(&env, "gurobi.log");

	// Ensure Gurobi behaves only as a very accurate linear programming solver.
	grb_error = GRBsetdblparam(env, GRB_DBL_PAR_BARCONVTOL, 0.0);
	grb_error = GRBsetdblparam(env, GRB_DBL_PAR_FEASIBILITYTOL, 1.0e-9);
	grb_error = GRBsetdblparam(env, GRB_DBL_PAR_HEURISTICS, 0.0);
	grb_error = GRBsetdblparam(env, GRB_DBL_PAR_MARKOWITZTOL, 0.999);
	grb_error = GRBsetdblparam(env, GRB_DBL_PAR_NODELIMIT, 0.0);
	grb_error = GRBsetdblparam(env, GRB_DBL_PAR_OPTIMALITYTOL, 1.0e-9);
	grb_error = GRBsetintparam(env, GRB_INT_PAR_CUTS, 0);
	grb_error = GRBsetintparam(env, GRB_INT_PAR_PRESOLVE, 0);
	grb_error = GRBsetintparam(env, GRB_INT_PAR_QUAD, 1);

	// Turn off Gurobi shell output.
	grb_error = GRBsetintparam(env, GRB_INT_PAR_OUTPUTFLAG, 0);

	// Set the timeout option.
	grb_error = GRBsetdblparam(env, GRB_DBL_PAR_TIMELIMIT, 60.0);

	// Read in the mixed-integer model.
	grb_error = GRBreadmodel(env, model_path.c_str(), &model);

	// Convert the read-in model to a linear program.
	SetupModel();
}

void Gomory::Optimize(void) {
	grb_error = GRBoptimize(model);

	int model_status;
	grb_error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &model_status);

	if (model_status != 2) {
		std::cout << INT_MAX << "," << INT_MAX << "," << INT_MAX << "," << INT_MAX << std::endl;
		std::exit(model_status);
	}
}

void Gomory::LexSimplex(void) {
	grb_error = GRBgetintattr(model, "NumConstrs", &num_constrs);

	double sol_i;
	grb_error = GRBgetdblattrelement(model, "X", 0, &sol_i);

	int id[1] = {0};
	double vid[1] = {1.0};
	grb_error = GRBaddconstr(model, 1, id, vid, GRB_EQUAL, sol_i, NULL);
	del_lex_constr_ids[0] = num_constrs;

	for (int j = 1; j < num_vars; j++) {
		for (int k = 0; k < num_vars; k++) {
			if (k == j) {
				grb_error = GRBsetdblattrelement(model, GRB_DBL_ATTR_OBJ, k, 1.0);
			} else {
				grb_error = GRBsetdblattrelement(model, GRB_DBL_ATTR_OBJ, k, 0.0);
			}
		}

		Optimize();

		int id[1] = {j};
		double vid[1] = {1.0};

		double sol_j;
		grb_error = GRBgetdblattrelement(model, "X", j, &sol_j);
		grb_error = GRBaddconstr(model, 1, id, vid, GRB_EQUAL, sol_j, NULL);
		del_lex_constr_ids[j] = num_constrs + j;
	}

	Optimize();

	// Restore the objective to that of the original problem.
	for (int j = 0; j < num_vars; j++) {
		grb_error = GRBsetdblattrelement(model, GRB_DBL_ATTR_OBJ, j, original_obj_coeffs[j]);
	}

	grb_error = GRBdelconstrs(model, num_vars, del_lex_constr_ids);
	Optimize();
}

void Gomory::SetupModel(void) {
	// Get important information about the original model.
	grb_error = GRBgetintattr(model, "NumVars", &num_vars);
	grb_error = GRBgetintattr(model, "NumIntVars", &num_int_vars);
	grb_error = GRBgetintattr(model, "NumConstrs", &num_constrs);
	original_num_constrs = num_constrs;

	// Allocate arrays to store integer variable ids and values.
	int_var_ids = (int*)malloc(num_int_vars*sizeof(int));
	int_var_vals = (double*)malloc(num_int_vars*sizeof(double));

	// Allocate arrays to store cut coefficient data.
	cut_coeff_ids = (int*)malloc(num_vars*sizeof(int));
	cut_coeff_vals = (double*)malloc(num_vars*sizeof(double));

	// Allocate arrays to be used with the lexicographic method.
	del_lex_constr_ids = (int*)malloc(num_vars*sizeof(int));
	original_obj_coeffs = (double*)malloc(num_vars*sizeof(double));

	// If a variable is not continuous, make it continuous.
	for (int j = 0, k = 0; j < num_vars; j++) {
		cut_coeff_ids[j] = j;

		char variable_type;
		grb_error = GRBgetcharattrelement(model, GRB_CHAR_ATTR_VTYPE, j, &variable_type);

		if (variable_type != GRB_CONTINUOUS) {
			grb_error = GRBsetcharattrelement(model, GRB_CHAR_ATTR_VTYPE, j, GRB_CONTINUOUS);
			int_var_ids[k++] = j;
		}
	}

	grb_error = GRBgetdblattrlist(model, GRB_DBL_ATTR_OBJ, num_vars,
	                              cut_coeff_ids, original_obj_coeffs);

	// Correctly size the matrices and vectors required for cuts.
	basis_size = num_vars;
	B.resize(basis_size, basis_size);
	B_inv.resize(basis_size, basis_size);
	r.resize(basis_size);
	c_beta.resize(basis_size);
	a_beta_r.resize(basis_size);

	// Set the total number of cuts used thus far to zero.
	num_cuts = 0;
}

int Gomory::UpdateVariableData(void) {
	grb_error = GRBgetdblattrlist(model, "X", num_int_vars, int_var_ids, int_var_vals);

	for (int k = 0; k < num_int_vars; k++) {
		if (fabs(int_var_vals[k] - round(int_var_vals[k])) >= away_epsilon) {
			// Add the variable to the set if it's fractional.
			frac_int_vars.insert(int_var_ids[k]);
		} else {
			// Remove the variable from the set if it's integer.
			frac_int_vars.erase(int_var_ids[k]);
		}
	}

	// Return the number of fractional variables.
	return (int)frac_int_vars.size();
}

void Gomory::UpdateBasisData(void) {
	grb_error = GRBgetintattr(model, "NumConstrs", &num_constrs);

	for (int i = 0, basis_count = 0; i < num_constrs; i++) {
		if (basis_count < basis_size) {
			int cbasis;
			grb_error = GRBgetintattrelement(model, "CBasis", i, &cbasis);

			if (cbasis == -1) {
				for (int j = 0; j < basis_size; j++) {
					double tmp_a_val;
					grb_error = GRBgetcoeff(model, i, j, &tmp_a_val);
					B(j, basis_count) = tmp_a_val;
				}

				double tmp_c_val;
				grb_error = GRBgetdblattrelement(model, "RHS", i, &tmp_c_val);
				c_beta(basis_count) = tmp_c_val;
				basis_count++;
			}
		} else {
			break;
		}
	}

	// Get the basis inverse.
	B_inv = B.inverse();
}

int Gomory::AddPureCut(int cut_var_index) {
	if (use_lex) {
		LexSimplex();
	}

	// Compute r.
	for (int i = 0; i < basis_size; i++) {
		r(i) = -floor(B_inv(i, cut_var_index));
	}
	
	// Compute constants for the cut.
	double c_beta_r = c_beta.transpose() * r;
	a_beta_r = B * r;

	for (int i = 0; i < num_vars; i++) {
		cut_coeff_vals[i] = round(a_beta_r(i));
	}

	cut_coeff_vals[cut_var_index] += 1.0;

	double y_bar_i;
	grb_error = GRBgetdblattrelement(model, "X", cut_var_index, &y_bar_i);
	double rhs = c_beta_r + floor(y_bar_i);
	grb_error = GRBaddconstr(model, num_vars, cut_coeff_ids, cut_coeff_vals,
	                         GRB_LESS_EQUAL, round(rhs), NULL);

	if (use_lex) {
		Optimize();
	}

	// Return number of generated cuts.
	return 1;
}

int Gomory::AddMixedCut(int cut_var_index) {
	if (use_lex) {
		LexSimplex();
	}

	for (int i = 0; i < basis_size; i++) {
		r(i) = fmax(0.0, -floor(B_inv(i, cut_var_index)));
	}

	double c_beta_r = c_beta.transpose() * r;
	a_beta_r = B * r;

	double y_bar_i;
	grb_error = GRBgetdblattrelement(model, "X", cut_var_index, &y_bar_i);

	for (int i = 0; i < num_vars; i++) {
		cut_coeff_vals[i] = a_beta_r(i);

		if (i == cut_var_index) {
			cut_coeff_vals[i] -= (y_bar_i - floor(y_bar_i) - 1.0);
		}
	}

	double rhs = -((y_bar_i - floor(y_bar_i) - 1.0) * floor(y_bar_i) - c_beta_r);
	grb_error = GRBaddconstr(model, num_vars, cut_coeff_ids, cut_coeff_vals,
	                         GRB_LESS_EQUAL, rhs, NULL);

	if (use_lex) {
		Optimize();
	}

	// Return the number of cuts generated.
	return 1;
}

int Gomory::AddPureRounds(void) {
	std::set<int>::iterator it;

	for (it = frac_int_vars.begin(); it != frac_int_vars.end(); ++it) {
		AddPureCut(*it);
	}

	return frac_int_vars.size();
}

int Gomory::AddMixedRounds(void) {
	std::set<int>::iterator it;

	for (it = frac_int_vars.begin(); it != frac_int_vars.end(); ++it) {
		AddMixedCut(*it);
	}

	return frac_int_vars.size();
}

int Gomory::GetLexIndex(void) {
	return *frac_int_vars.begin();
}

int Gomory::GetRandomIndex(void) {
	int set_index = rand() % frac_int_vars.size();

	std::set<int>::const_iterator it = frac_int_vars.begin();
	if (set_index != 0) {
		advance(it, set_index - 1);
	}

	return *it;
}

int Gomory::PurgeCuts(void) {
	grb_error = GRBgetintattr(model, "NumConstrs", &num_constrs);
	std::vector<int> purge_cut_ids;

	for (int i = original_num_constrs; i < num_constrs; i++) {
		double constraint_slack;
		grb_error = GRBgetdblattrelement(model, "Slack", i, &constraint_slack);

		if (fabs(constraint_slack) > purge_epsilon) {
			purge_cut_ids.push_back(i);
		}
	}

	grb_error = GRBdelconstrs(model, (int)purge_cut_ids.size(), purge_cut_ids.data());
	Optimize();

	return purge_cut_ids.size();
}

void Gomory::PrintStep(void) {
	static bool started_print = false;
	if (started_print == false) {
		started_print = true;
		std::cout << "ncuts,nconstrs,det,obj" << std::endl;
	}

	double current_obj;
	grb_error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &current_obj);
	std::cout << num_cuts << "," << num_constrs << "," << B.determinant()
	          << "," << current_obj << std::endl;
}

int Gomory::Step(void) {
	if (objective_value != old_objective_value && use_purging) {
		PurgeCuts();
		old_objective_value = objective_value;
	}

	UpdateBasisData();

	if (num_vars == num_int_vars) {
		if (use_rounds) {
			num_cuts += use_fgmi ? AddMixedRounds() : AddPureRounds();
		} else {
			int cut_var_id = use_lex ? GetLexIndex() : GetRandomIndex();
			num_cuts += use_fgmi ? AddMixedCut(cut_var_id) : AddPureCut(cut_var_id);
		}
	} else {
		if (use_rounds) {
			num_cuts += AddMixedRounds();
		} else {
			int cut_var_id = use_lex ? GetLexIndex() : GetRandomIndex();
			num_cuts += AddMixedCut(cut_var_id);
		}
	}

	Optimize();
	grb_error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objective_value);

	return UpdateVariableData();
}

void Gomory::Run(void) {
	Optimize();
	int num_frac_vars = UpdateVariableData();
	UpdateBasisData();
	PrintStep();

	while (num_frac_vars > 0 && num_cuts < max_cuts) {
		num_frac_vars = Step();
		PrintStep();
	}

	grb_error = GRBwrite(model, solution_path.c_str());
}

Gomory::~Gomory(void) {
	free(cut_coeff_vals);
	free(cut_coeff_ids);
	free(int_var_vals);
	free(int_var_ids);
	free(del_lex_constr_ids);
	free(original_obj_coeffs);
}
