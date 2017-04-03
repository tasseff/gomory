#include <iostream>
#include "gomory_rounds.h"

GomoryRounds::GomoryRounds(const rapidjson::Value& root) : GomoryNaive(root) { }

int GomoryRounds::AddPureRounds(void) {
	std::set<int>::iterator it;
	for (it = frac_int_vars.begin(); it != frac_int_vars.end(); ++it) {
		int cut_var_index = *it;
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
	}
	return frac_int_vars.size();
}

int GomoryRounds::AddMixedRounds(void) {
	std::set<int>::iterator it;
	for (it = frac_int_vars.begin(); it != frac_int_vars.end(); ++it) {
		int cut_var_index = *it;

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

		double rhs = -((y_bar_i - floor(y_bar_i) - 1.0) * floor(y_bar_i) -
									 c_beta_r);
		grb_error = GRBaddconstr(model, num_vars, cut_coeff_ids, cut_coeff_vals,
			GRB_LESS_EQUAL, rhs, NULL);
	}
	return frac_int_vars.size();
}

int GomoryRounds::Step(void) {
	UpdateBasisData();

	if (num_vars == num_int_vars) {
		// If all of the variables were integer, use the pure integer cut.
		num_cuts += AddPureRounds();
	} else {
		// Otherwise, use the mixed-integer cut.
		num_cuts += AddMixedRounds();
	}

	grb_error = GRBoptimize(model);

	return UpdateVariableData();
}

void GomoryRounds::Run(void) {
	grb_error = GRBoptimize(model);
	int num_frac_vars = UpdateVariableData();

	while (num_frac_vars > 0) {
		num_frac_vars = Step();
		PrintStep();
	}

	grb_error = GRBwrite(model, solution_path.c_str());
}
