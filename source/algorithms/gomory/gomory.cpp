#include <iostream>
#include <iterator>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unordered_set>
#include <common/document.h>
#include <common/error.h>
#include <common/file.h>
#include <Eigen/Dense>
#include <stdio.h>
#include <stdlib.h>
#include "gomory.h"

Gomory::Gomory(const rapidjson::Value& root) {
	std::string model_path = root["model"].GetString();
	epsilon = root["epsilon"].GetDouble();
	grb_error = GRBloadenv(&env, "gurobi.log");

	if (root["gurobiPresolve"].GetBool() == false)
		grb_error = GRBsetintparam(env, GRB_INT_PAR_PRESOLVE, 0);
	if (root["gurobiCuts"].GetBool() == false)
		grb_error = GRBsetintparam(env, GRB_INT_PAR_CUTS, 0);
	if (root["gurobiOutput"].GetBool() == false)
		grb_error = GRBsetintparam(env, GRB_INT_PAR_OUTPUTFLAG, 0);
	if (root["gurobiBB"].GetBool() == false)
		grb_error = GRBsetdblparam(env, GRB_DBL_PAR_NODELIMIT, 0.0);
	if (root["gurobiHeuristics"].GetBool() == false)
		grb_error = GRBsetdblparam(env, GRB_DBL_PAR_HEURISTICS, 0.0);

	grb_error = GRBreadmodel(env, model_path.c_str(), &model);
}

Gomory::~Gomory(void) {
	free(Binv_tmp->ind);
	free(Binv_tmp->val);
	free(Binv_tmp);
	free(svec_tmp->ind);
	free(svec_tmp->val);
	free(svec_tmp);
	GRBfreemodel(model);
	GRBfreeenv(env);
}

void Gomory::Run(void) {
	// Keep track of the variables that were originally integer.
	grb_error = GRBgetintattr(model, "NumIntVars", &num_int_vars);
	int* int_var_ids = new int[num_int_vars];
	double* int_var_vals = new double[num_int_vars];

	// Keep track of integer variables that are fractional in the relaxation.
	std::unordered_set<unsigned int> frac_var_ids;

	// Convert integer variables to continuous variables.
	grb_error = GRBgetintattr(model, "NumVars", &num_vars);
	grb_error = GRBgetintattr(model, "NumConstrs", &num_constrs);

	//// Preallocate things related to the basis.
	unsigned int basis_size = num_vars;
	int* basis_head = new int[basis_size];
	//double* a_beta_r = new double[basis_size];

	//// Preallocate things related to r.
	//int* r_indices = new int[basis_size];
	//double* r_vals = new double[basis_size];
	//GRBsvec r = {basis_size, r_indices, r_vals};

	// Preallocate things related to Binv_tmp.
	//int* Binv_tmp_indices = new int[100*basis_size];
	//double* Binv_tmp_vals = new double[100*basis_size];

	Binv_tmp = (GRBsvec*)malloc(sizeof(GRBsvec));
	Binv_tmp->len = basis_size;
	Binv_tmp->ind = (int*)malloc(basis_size*sizeof(int)); //         new int[100*basis_size]; //Binv_tmp_indices;
	Binv_tmp->val = (double*)malloc(basis_size*sizeof(double));//             new double[100*basis_size]; //Binv_tmp_vals;

	svec_tmp = (GRBsvec*)malloc(sizeof(GRBsvec));
	svec_tmp->len = 1;
	svec_tmp->ind = (int*)malloc(1*sizeof(int)); //   new int[1]; //Binv_tmp_indices;
	svec_tmp->val = (double*)malloc(1*sizeof(double)); //new double[1]; //Binv_tmp_vals;
	svec_tmp->val[0] = 1.0;

	//GRBsvec* Binv_tmp = new GRBsvec;
	//Binv_tmp = {basis_size, Binv_tmp_indices, Binv_tmp_vals};

	// Variables used to construct a single new constraint.
	int* cind = new int[num_vars];
	double* cval = new double[num_vars];
	double rhs;

	// Preallocate the svec used to obtain a portion of the basis.
	//int* svec_tmp_ids = new int[1];
	//double* svec_tmp_vals = new double[1];
	//svec_tmp_vals[0] = 1.0;

	//GRBsvec svec_tmp = {1, svec_tmp_ids, svec_tmp_vals};

	Eigen::MatrixXd B(basis_size, basis_size);
	Eigen::MatrixXd Binv(basis_size, basis_size);
	Eigen::VectorXd r(basis_size);
	Eigen::VectorXd c_beta(basis_size);
	Eigen::VectorXd a_beta_r(basis_size);

	char variable_type;
	unsigned int num_cuts = 0;
	for (unsigned int j = 0, k = 0; j < num_vars; j++) {
		cind[j] = j;

		// If the variable type is not continuous, make it continuous.
		grb_error = GRBgetcharattrelement(model, "VType", j, &variable_type);
		if (variable_type != 'C') {
			grb_error = GRBsetcharattrelement(model, "VType", j, GRB_CONTINUOUS);
			int_var_ids[k++] = j;
		}
	}

	while (true) {
		grb_error = GRBoptimize(model);
		grb_error = GRBgetdblattrlist(model, "X", num_int_vars, int_var_ids, int_var_vals);

		for (unsigned int k = 0; k < num_int_vars; k++) {
			// Add the variable to the set if it's fractional.
			if (fabs(int_var_vals[k] - floor(int_var_vals[k] + 0.5)) > epsilon) {
				frac_var_ids.insert(int_var_ids[k]); // TODO: Change to emplace when using gcc >= 4.8.
			} else {
				frac_var_ids.erase(int_var_ids[k]); // Is this valid if the value isn't in the set?
			}
		}

		// If there are no fractional variables, exit the algorithm.
		if (frac_var_ids.size() == 0) {
			break;
		}

		// Set the cutting variable index.
		int cut_var_index = *frac_var_ids.begin();

		// Get the basis inverse.
		// TODO: Is there an even faster way to get the basis inverse?
		grb_error = GRBgetBasisHead(model, basis_head);
		for (unsigned int j = 0; j < basis_size; j++) {
			cval[j] = 0.0;
			//svec_tmp->ind[0] = j;
			//grb_error = GRBBSolve(model, svec_tmp, Binv_tmp);
			//grb_error = GRBBSolve(model, svec_tmp, Binv_tmp);
			//std::cout << svec_tmp->ind[0] << std::endl;
			//if (grb_error != 0)
			//	std::cout << grb_error << std::endl;
		//	for (unsigned int i = 0; i < basis_size; i++) {
		//		Binv(i, j) = Binv_tmp_vals[i];
		//	}
		}

		//grb_error = GRBoptimize(model);

		std::cout << std::endl;

		// Get the basis.
		grb_error = GRBgetintattr(model, "NumConstrs", &num_constrs);
		for (unsigned int i = 0, basis_count = 0; i < num_constrs; i++) {
			if (basis_count < basis_size) {
				int cbasis;
				grb_error = GRBgetintattrelement(model, "CBasis", i, &cbasis);
				if (cbasis == -1) {
					for (unsigned int j = 0; j < basis_size; j++) {
						double tmp_a_val;
						grb_error = GRBgetcoeff(model, i, j, &tmp_a_val);
						B(j, basis_count) = tmp_a_val;
					}

					double tmp_c_val;
					grb_error = GRBgetdblattrelement(model, "RHS", i, &tmp_c_val);
					c_beta(basis_count) = tmp_c_val;
					basis_count++;
				}
			} else {
				break;
			}
		}

		Binv = B.inverse();

		// put a one in the correct position of e_i, will remove at end of iteration
		for (unsigned int i = 0; i < basis_size; i++) {
			r(i) = -floor(Binv(i, cut_var_index));
		}
		
		// get constants ready for cut
		double c_beta_r = c_beta.transpose()*r;
		
		// create linear expression with the variables
		a_beta_r = B * r;

		for (unsigned int i = 0; i < num_vars; i++) {
			cval[i] = a_beta_r(i);
		}

		cval[cut_var_index] += 1.0;

		double y_bar_i;
		grb_error = GRBgetdblattrelement(model, "X", cut_var_index, &y_bar_i);
		rhs = c_beta_r + floor(y_bar_i);
		grb_error = GRBaddconstr(model, num_vars, cind, cval, GRB_LESS_EQUAL, rhs, NULL);

		num_cuts++;
	}

	int optimstatus;
	grb_error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);

	if (optimstatus == GRB_OPTIMAL) {
		double objval;
		grb_error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
		std::cout << "Optimal objective: " << objval << std::endl;
	} else if (optimstatus == GRB_INF_OR_UNBD) {
		std::cout << "Model is infeasible or unbounded" << std::endl;
	} else if (optimstatus == GRB_INFEASIBLE) {
		std::cout << "Model is infeasible" << std::endl;
	} else if (optimstatus == GRB_UNBOUNDED) {
		std::cout << "Model is unbounded" << std::endl;
	} else {
		std::cout << "Optimization was stopped with status = " << optimstatus << std::endl;
	}

	std::cout << "Number of cuts added: " << num_cuts << std::endl;

	//delete[] int_var_vals;
	//delete[] int_var_ids;
	//delete[] cind;
	//delete[] cval;
}

int main(int argc, char* argv[]) {
	// Check if a JSON file has been specified.
	if (argc <= 1) {
		PrintErrorAndExit("JSON file has not been specified.");
	}

	// Read the scenario file.
	File model(argv[1]);

	// Parse the scenario file as a JSON document.
	Document json(model);

	// Set up the model.
	Gomory gomory(json.root["parameters"]);

	// Run the model.
	gomory.Run();

	// Program executed successfully.
	return 0;
}
