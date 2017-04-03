#pragma once

#include "gomory_naive.h"

class GomoryRoundsLex : public GomoryNaive {
public:
  GomoryRoundsLex(const rapidjson::Value& root);
  ~GomoryRoundsLex(void);
  void Run(void);
  int Step(void);

protected:
  int AddPureRounds(void);
  int AddMixedRounds(void);
  void LexSimplex(void);
  double* original_obj_coeffs; // Original objective coefficients.
  int* del_lex_constr_ids; // Indices of constraints to delete after lex step.
  };
