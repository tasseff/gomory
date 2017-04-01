#include <iostream>
#include "gomory_lex.h"

GomoryLex::GomoryLex(const rapidjson::Value& root) : GomoryNaive(root) {
	original_obj_coeffs = (double*)malloc(num_vars*sizeof(double));
	del_lex_constr_ids = (int*)malloc(num_vars*sizeof(int));
	grb_error = GRBgetdblattrlist(model, GRB_DBL_ATTR_OBJ, num_vars,
	                              cut_coeff_ids, original_obj_coeffs);
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
	grb_error = GRBdelconstrs(model, num_vars, del_lex_constr_ids);

	// Restore the objective to that of the original problem.
	for (unsigned int j = 0; j < num_vars; j++) {
		grb_error = GRBsetdblattrelement(model, GRB_DBL_ATTR_OBJ, j, original_obj_coeffs[j]);
	}

	grb_error = GRBoptimize(model);
}

void GomoryLex::Run(void) {
	grb_error = GRBoptimize(model);
	int num_frac_vars = UpdateVariableData();

	while (num_frac_vars > 0) {
		LexSimplex();
		num_frac_vars = Step();
	}

	grb_error = GRBwrite(model, solution_path.c_str());
}
