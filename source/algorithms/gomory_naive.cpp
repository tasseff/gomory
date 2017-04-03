#include <cstdlib>
#include <iostream>
#include <limits>
#include "gomory_naive.h"

#define AWAY 1.0e-2

GomoryNaive::GomoryNaive(const rapidjson::Value& root) : BaseModel(root) {
	// Ensure Gurobi behaves only as a very accurate linear programming solver.
	grb_error = GRBsetdblparam(env, GRB_DBL_PAR_BARCONVTOL, 0.0);
	grb_error = GRBsetdblparam(env, GRB_DBL_PAR_FEASIBILITYTOL, 1.0e-9);
	grb_error = GRBsetdblparam(env, GRB_DBL_PAR_HEURISTICS, 0.0);
	grb_error = GRBsetdblparam(env, GRB_DBL_PAR_MARKOWITZTOL, 0.999);
	grb_error = GRBsetdblparam(env, GRB_DBL_PAR_NODELIMIT, 0.0);
	grb_error = GRBsetdblparam(env, GRB_DBL_PAR_OPTIMALITYTOL, 1.0e-9);
	grb_error = GRBsetintparam(env, GRB_INT_PAR_CUTS, 0);
	grb_error = GRBsetintparam(env, GRB_INT_PAR_OUTPUTFLAG, 0);
	grb_error = GRBsetintparam(env, GRB_INT_PAR_PRESOLVE, 0);
	grb_error = GRBsetintparam(env, GRB_INT_PAR_QUAD, 1);

	// Read in the mixed-integer model.
	std::string model_path = root["model"].GetString();
	grb_error = GRBreadmodel(env, model_path.c_str(), &model);

	// Convert the read-in model to a linear program.
	SetupModel();
}

void GomoryNaive::SetupModel(void) {
	// Get important information about the original model.
	grb_error = GRBgetintattr(model, "NumVars", &num_vars);
	grb_error = GRBgetintattr(model, "NumIntVars", &num_int_vars);
	grb_error = GRBgetintattr(model, "NumConstrs", &num_constrs);

	// Allocate arrays to store integer variable ids and values.
	int_var_ids = (int*)malloc(num_int_vars*sizeof(int));
	int_var_vals = (double*)malloc(num_int_vars*sizeof(double));

	// Allocate arrays to store cut coefficient data.
	cut_coeff_ids = (int*)malloc(num_vars*sizeof(int));
	cut_coeff_vals = (double*)malloc(num_vars*sizeof(double));

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

int GomoryNaive::UpdateVariableData(void) {
	grb_error = GRBgetdblattrlist(model, "X", num_int_vars, int_var_ids, int_var_vals);

	for (int k = 0; k < num_int_vars; k++) {
		if (fabs(int_var_vals[k] - round(int_var_vals[k])) >= AWAY) {
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

void GomoryNaive::UpdateBasisData(void) {
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
	//B = B.transpose().eval();
	//std::cout << B << std::endl;
	B_inv = B.inverse();
}

int GomoryNaive::AddPureCut(int cut_var_index) {
	// Compute r.
	for (unsigned int i = 0; i < basis_size; i++) {
		r(i) = -floor(B_inv(i, cut_var_index));
	}
	
	// Compute constants for the cut.
	double c_beta_r = c_beta.transpose() * r;
	a_beta_r = B * r;

	for (unsigned int i = 0; i < num_vars; i++) {
		cut_coeff_vals[i] = a_beta_r(i);
	}

	cut_coeff_vals[cut_var_index] += 1.0;

	double y_bar_i;
	grb_error = GRBgetdblattrelement(model, "X", cut_var_index, &y_bar_i);
	double rhs = c_beta_r + floor(y_bar_i);
	grb_error = GRBaddconstr(model, num_vars, cut_coeff_ids, cut_coeff_vals,
	                         GRB_LESS_EQUAL, rhs, NULL);

	// Return number of generated cuts.
	return 1;
}

int GomoryNaive::AddMixedCut(int cut_var_index) {
	for (unsigned int i = 0; i < basis_size; i++) {
		r(i) = fmax(0.0, -floor(B_inv(i, cut_var_index)));
	}

	double c_beta_r = c_beta.transpose() * r;
	a_beta_r = B * r;

	double y_bar_i;
	grb_error = GRBgetdblattrelement(model, "X", cut_var_index, &y_bar_i);

	for (unsigned int i = 0; i < num_vars; i++) {
		cut_coeff_vals[i] = a_beta_r(i);
		if (i == cut_var_index) {
			cut_coeff_vals[i] -= (y_bar_i - floor(y_bar_i) - 1.0);
		}
	}

	double rhs = -((y_bar_i - floor(y_bar_i) - 1.0) * floor(y_bar_i) - c_beta_r);
	grb_error = GRBaddconstr(model, num_vars, cut_coeff_ids, cut_coeff_vals,
	                         GRB_LESS_EQUAL, rhs, NULL);

	// Return the number of cuts generated.
	return 1;
}

int GomoryNaive::GetRandomIndex(void) {
	int set_index = rand() % frac_int_vars.size();

	std::set<int>::const_iterator it = frac_int_vars.begin();
	if (set_index != 0) {
		advance(it, set_index - 1);
	}

	return *it;
}

int GomoryNaive::GetMostFractionalIndex(void) {
	double max_diff = 0.0;
	int best_index = *frac_int_vars.begin();

	for (std::set<int>::const_iterator it = ++frac_int_vars.begin(); it != frac_int_vars.end(); it++) {
		double val;
		grb_error = GRBgetdblattrelement(model, "X", *it, &val);

		double closest_int = round(val);
		double diff = fabs(val - closest_int);
		max_diff = diff > max_diff ? diff : max_diff;
		best_index = diff > max_diff ? *it : best_index;
	}

	return best_index;
}

int GomoryNaive::GetLeastFractionalIndex(void) {
	double min_diff = std::numeric_limits<double>::max();
	int best_index = *frac_int_vars.begin();

	for (std::set<int>::const_iterator it = ++frac_int_vars.begin(); it != frac_int_vars.end(); it++) {
		double val;
		grb_error = GRBgetdblattrelement(model, "X", *it, &val);

		double closest_int = round(val);
		double diff = fabs(val - closest_int);

		if (diff > AWAY && diff < min_diff) {
			min_diff = diff;
			best_index = *it;
		}
	}

	return best_index;
}

void GomoryNaive::PrintStep(void) {
	double B_det = B.determinant();
	double current_obj;
	grb_error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &current_obj);
	std::cout << num_cuts << "\t" << B.determinant() << "\t" << current_obj << std::endl;
}

int GomoryNaive::Step(void) {
	UpdateBasisData();
	//int cut_id = *frac_int_vars.begin();
	int cut_id = GetRandomIndex();

	if (num_vars == num_int_vars) {
		// If all of the variables were integer, use the pure integer cut.
		num_cuts += AddPureCut(cut_id);
		//num_cuts += AddMixedCut(cut_id);
	} else {
		// Otherwise, use the mixed-integer cut.
		num_cuts += AddMixedCut(cut_id);
	}

	grb_error = GRBoptimize(model);
	int num_frac_vars = UpdateVariableData();

	return num_frac_vars;
}

void GomoryNaive::Run(void) {
	grb_error = GRBoptimize(model);
	int num_frac_vars = UpdateVariableData();

	while (num_frac_vars > 0) {
		num_frac_vars = Step();
		PrintStep();
	}

	grb_error = GRBwrite(model, solution_path.c_str());
}

GomoryNaive::~GomoryNaive(void) {
	free(cut_coeff_vals);
	free(cut_coeff_ids);
	free(int_var_vals);
	free(int_var_ids);
}
