#include <iostream>
#include "gomory_rounds_lex.h"

int GomoryRoundsLex::AddPureRounds(void) {
	std::set<int>::iterator it;
	for (it = frac_int_vars.begin(); it != frac_int_vars.end(); ++it) {
		LexSimplex();
		AddPureCut(*it);
		grb_error = GRBoptimize(model);
	}

	return frac_int_vars.size();
}

int GomoryRoundsLex::AddMixedRounds(void) {
	std::set<int>::iterator it;
	for (it = frac_int_vars.begin(); it != frac_int_vars.end(); ++it) {
		LexSimplex();
		AddMixedCut(*it);
		grb_error = GRBoptimize(model);
	}

	return frac_int_vars.size();
}

int GomoryRoundsLex::Step(void) {
	if (objective_value != old_objective_value) {
		iter_since_purge = 0;
		int num_constrs_purged = PurgeCuts();
		old_objective_value = objective_value;
	}

	UpdateBasisData();

	if (num_vars == num_int_vars) {
		// If all of the variables were integer, use the pure integer cut.
		num_cuts += AddPureRounds();
		//num_cuts += AddMixedRounds();
	} else {
		// Otherwise, use the mixed-integer cut.
		num_cuts += AddMixedRounds();
	}

	grb_error = GRBoptimize(model);
	grb_error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objective_value);
	iter_since_purge++;

	return UpdateVariableData();
}

GomoryRoundsLex::GomoryRoundsLex(const rapidjson::Value& root) : GomoryLex(root) {}

void GomoryRoundsLex::Run(void) {
	grb_error = GRBoptimize(model);
	grb_error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objective_value);
	old_objective_value = objective_value;
	int num_frac_vars = UpdateVariableData();
	iter_since_purge = 0;

	while (num_frac_vars > 0 && num_cuts < MAX_CUTS) {
		num_frac_vars = Step();
		PrintStep();
	}

	grb_error = GRBwrite(model, solution_path.c_str());
}
