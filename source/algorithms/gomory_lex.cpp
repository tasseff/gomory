#include <iostream>
#include <vector>
#include "gomory_lex.h"

GomoryLex::GomoryLex(const rapidjson::Value& root) : GomoryNaive(root) {
	original_obj_coeffs = (double*)malloc(num_vars*sizeof(double));
	del_lex_constr_ids = (int*)malloc(num_vars*sizeof(int));
	grb_error = GRBgetdblattrlist(model, GRB_DBL_ATTR_OBJ, num_vars,
	                              cut_coeff_ids, original_obj_coeffs);
	grb_error = GRBgetintattr(model, "NumConstrs", &original_num_constrs);
}

GomoryLex::~GomoryLex(void) {
	free(del_lex_constr_ids);
	free(original_obj_coeffs);
}

void GomoryLex::LexSimplex(void) {
	grb_error = GRBgetintattr(model, "NumConstrs", &num_constrs);

	double sol_i;
	grb_error = GRBgetdblattrelement(model, "X", 0, &sol_i);

	int id[1] = {0};
	double vid[1] = {1.0};
	grb_error = GRBaddconstr(model, 1, id, vid, GRB_EQUAL, sol_i, NULL);
	del_lex_constr_ids[0] = num_constrs;

	for (unsigned int j = 1; j < num_vars; j++) {
		for (unsigned int k = 0; k < num_vars; k++) {
			if (k == j) {
				grb_error = GRBsetdblattrelement(model, GRB_DBL_ATTR_OBJ, k, 1.0);
			} else {
				grb_error = GRBsetdblattrelement(model, GRB_DBL_ATTR_OBJ, k, 0.0);
			}
		}

		grb_error = GRBoptimize(model);

		int id[1] = {j};
		double vid[1] = {1.0};

		double sol_j;
		grb_error = GRBgetdblattrelement(model, "X", j, &sol_j);
		grb_error = GRBaddconstr(model, 1, id, vid, GRB_EQUAL, sol_j, NULL);
		del_lex_constr_ids[j] = num_constrs + j;
	}

	grb_error = GRBoptimize(model);

	// Restore the objective to that of the original problem.
	for (unsigned int j = 0; j < num_vars; j++) {
		grb_error = GRBsetdblattrelement(model, GRB_DBL_ATTR_OBJ, j, original_obj_coeffs[j]);
	}

	grb_error = GRBdelconstrs(model, num_vars, del_lex_constr_ids);
	grb_error = GRBoptimize(model);
}

int GomoryLex::PurgeCuts(void) {
	grb_error = GRBgetintattr(model, "NumConstrs", &num_constrs);
	std::vector<int> purge_cut_ids;

	for (int i = original_num_constrs; i < num_constrs; i++) {
		double constraint_slack;
		grb_error = GRBgetdblattrelement(model, "Slack", i, &constraint_slack);

		if (fabs(constraint_slack) > 1.0e-9) {
			purge_cut_ids.push_back(i);
		}
	}

	grb_error = GRBdelconstrs(model, (int)purge_cut_ids.size(), purge_cut_ids.data());
	grb_error = GRBoptimize(model);
	return purge_cut_ids.size();
}

int GomoryLex::Step(void) {
	if (objective_value != old_objective_value) {
		iter_since_purge = 0;
		int num_constrs_purged = PurgeCuts();
		old_objective_value = objective_value;
	}

	UpdateBasisData();
	//int cut_id = GetLeastFractionalIndex();
	//int cut_id = GetMostFractionalIndex();
	int cut_id = GetRandomIndex();
	//int cut_id = *frac_int_vars.begin();

	if (num_vars == num_int_vars) {
		// If all of the variables were integer, use the pure integer cut.
		LexSimplex();
		num_cuts += AddPureCut(cut_id);
	} else {
		// Otherwise, use the mixed-integer cut.
		LexSimplex();
		num_cuts += AddMixedCut(cut_id);
	}

	grb_error = GRBoptimize(model);
	grb_error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objective_value);
	iter_since_purge++;

	return UpdateVariableData();
}

void GomoryLex::Run(void) {
	grb_error = GRBoptimize(model);
	grb_error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objective_value);
	old_objective_value = objective_value;
	int num_frac_vars = UpdateVariableData();
	iter_since_purge = 0;

	while (num_frac_vars > 0) {
		num_frac_vars = Step();
		PrintStep();
	}

	grb_error = GRBwrite(model, solution_path.c_str());
}
