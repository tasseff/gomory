#include <iostream>
#include "gomory_rounds_lex.h"

int GomoryRoundsLex::AddPureRounds(void) {
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

int GomoryRoundsLex::AddMixedRounds(void) {
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

void GomoryRoundsLex::LexSimplex(void) {
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

GomoryRoundsLex::GomoryRoundsLex(const rapidjson::Value& root) : GomoryNaive(root) {
  original_obj_coeffs = (double*)malloc(num_vars*sizeof(double));
  del_lex_constr_ids = (int*)malloc(num_vars*sizeof(int));
  grb_error = GRBgetdblattrlist(model, GRB_DBL_ATTR_OBJ, num_vars,
    cut_coeff_ids, original_obj_coeffs);
}

GomoryRoundsLex::~GomoryRoundsLex(void) {
  free(del_lex_constr_ids);
  free(original_obj_coeffs);
}

void GomoryRoundsLex::Run(void) {
  grb_error = GRBoptimize(model);
  int num_frac_vars = UpdateVariableData();

  while (num_frac_vars > 0) {
    num_frac_vars = Step();
    PrintStep();
  }

  grb_error = GRBwrite(model, solution_path.c_str());
}
