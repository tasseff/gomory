#pragma once

#include "gomory_naive.h"

class GomoryLex : public GomoryNaive {
public:
	GomoryLex(const rapidjson::Value& root);
	~GomoryLex(void);
	void Run(void);

protected:
	void LexSimplex(void);
	double* original_obj_coeffs; // Original objective coefficients.
	int* del_lex_constr_ids; // Indices of constraints to delete after lex step.
};
