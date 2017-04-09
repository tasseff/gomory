#pragma once

#include "gomory_naive.h"

class GomoryLex : public GomoryNaive {
public:
	GomoryLex(const rapidjson::Value& root);
	~GomoryLex(void);
	void Run(void);
	int PurgeCuts(void);
	int Step(void);

protected:
	int LexSimplex(void);
	int original_num_constrs;
	int iter_since_purge;
	double objective_value;
	double old_objective_value;
	double* original_obj_coeffs; // Original objective coefficients.
	int* del_lex_constr_ids; // Indices of constraints to delete after lex step.
};
