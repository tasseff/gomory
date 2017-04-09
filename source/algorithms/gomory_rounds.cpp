#include <iostream>
#include "gomory_rounds.h"

GomoryRounds::GomoryRounds(const rapidjson::Value& root) : GomoryNaive(root) { }

int GomoryRounds::AddPureRounds(void) {
	std::set<int>::iterator it;

	for (it = frac_int_vars.begin(); it != frac_int_vars.end(); ++it) {
		AddPureCut(*it);
	}

	return frac_int_vars.size();
}

int GomoryRounds::AddMixedRounds(void) {
	std::set<int>::iterator it;
	for (it = frac_int_vars.begin(); it != frac_int_vars.end(); ++it) {
		AddMixedCut(*it);
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
  int status;
  grb_error = GRBgetintattr(model, "Status", &status);
  if(status == 2) {
    return UpdateVariableData();
  }
  std::cout << INT_MAX << "," << INT_MAX << "," << INT_MAX << "," << INT_MAX << std::endl;
  return -1;
}

void GomoryRounds::Run(void) {
	grb_error = GRBoptimize(model);
	int num_frac_vars = UpdateVariableData();
	PrintStep();
	while (num_frac_vars > 0 && num_cuts < MAX_CUTS) {
		num_frac_vars = Step();
    if(num_frac_vars >= 0) {
		  PrintStep();
    }
	}

	grb_error = GRBwrite(model, solution_path.c_str());
}
