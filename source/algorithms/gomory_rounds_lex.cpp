#include <iostream>
#include "gomory_rounds_lex.h"

int GomoryRoundsLex::AddPureRounds(void) {
  std::set<int>::iterator it;
  for (it = frac_int_vars.begin(); it != frac_int_vars.end(); ++it) {
    AddPureCut(*it);
  }
  return frac_int_vars.size();
}

int GomoryRoundsLex::AddMixedRounds(void) {
  std::set<int>::iterator it;
  for (it = frac_int_vars.begin(); it != frac_int_vars.end(); ++it) {
    AddMixedCut(*it);
  }
  return frac_int_vars.size();
}

int GomoryRoundsLex::Step(void) {
  UpdateBasisData();
  if (num_vars == num_int_vars) {
    // If all of the variables were integer, use the pure integer cut.
    LexSimplex();
    num_cuts += AddPureRounds();
  } else {
    // Otherwise, use the mixed-integer cut.
    LexSimplex();
    num_cuts += AddMixedRounds();
  }

  grb_error = GRBoptimize(model);

  return UpdateVariableData();
}

GomoryRoundsLex::GomoryRoundsLex(const rapidjson::Value& root) : GomoryLex(root) {}

void GomoryRoundsLex::Run(void) {
  grb_error = GRBoptimize(model);
  int num_frac_vars = UpdateVariableData();

  while (num_frac_vars > 0) {
    num_frac_vars = Step();
    PrintStep();
  }

  grb_error = GRBwrite(model, solution_path.c_str());
}
