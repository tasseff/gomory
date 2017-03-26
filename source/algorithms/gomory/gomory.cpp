#include <iostream>
#include <iterator>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <limits>
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

unsigned int Gomory::AddPureCut(int cut_var_index) {
	// Compute r.
	for (unsigned int i = 0; i < basis_size; i++) {
		r(i) = -floor(Binv(i, cut_var_index));
	}
	
	// Compute constants for the cut.
	double c_beta_r = c_beta.transpose() * r;
	a_beta_r = B * r;

	for (unsigned int i = 0; i < num_vars; i++) {
		cval[i] = a_beta_r(i);
	}

	cval[cut_var_index] += 1.0;

	double y_bar_i;
	grb_error = GRBgetdblattrelement(model, "X", cut_var_index, &y_bar_i);
	double rhs = c_beta_r + floor(y_bar_i);
	grb_error = GRBaddconstr(model, num_vars, cind, cval, GRB_LESS_EQUAL, rhs, NULL);

	// Return number of cuts generated.
	return 1;
}

unsigned int Gomory::AddMixedCut(int cut_var_index) {
	for (unsigned int i = 0; i < basis_size; i++) {
		r(i) = fmax(0.0, -floor(Binv(i, cut_var_index)));
	}

	double c_beta_r = c_beta.transpose() * r;
	a_beta_r = B * r;

	double y_bar_i;
	grb_error = GRBgetdblattrelement(model, "X", cut_var_index, &y_bar_i);

	for (unsigned int i = 0; i < num_vars; i++) {
		cval[i] = a_beta_r(i);
		if (i == cut_var_index) {
			cval[i] -= (y_bar_i - floor(y_bar_i) - 1.0);
		}
	}

	double rhs = (y_bar_i - floor(y_bar_i) - 1.0) * floor(y_bar_i) - c_beta_r;
	grb_error = GRBaddconstr(model, num_vars, cind, cval, GRB_LESS_EQUAL, -rhs, NULL);

	return 1; // Return the number of cuts generated.
}

Gomory::~Gomory(void) {
	GRBfreemodel(model);
	GRBfreeenv(env);
}

void Gomory::Run(void) {
  std::random_device rd;     // only used once to initialise (seed) engine
  std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
	// Keep track of the variables that were originally integer.
	grb_error = GRBgetintattr(model, "NumIntVars", &num_int_vars);
	int* int_var_ids = (int*)malloc(num_int_vars*sizeof(int));
	double* int_var_vals = (double*)malloc(num_int_vars*sizeof(double));

	// Keep track of integer variables that are fractional in the relaxation.
	std::unordered_set<unsigned int> frac_var_ids;

	// Convert integer variables to continuous variables.
	grb_error = GRBgetintattr(model, "NumVars", &num_vars);
	grb_error = GRBgetintattr(model, "NumConstrs", &num_constrs);

	// Preallocate things related to the basis.
	basis_size = num_vars;

	// Variables used to construct a single new constraint.
	cind = (int*)malloc(num_vars*sizeof(int)); //   new int[num_vars];
	cval = (double*)malloc(num_vars*sizeof(double)); // new double[num_vars];

	B.resize(basis_size, basis_size);
	Binv.resize(basis_size, basis_size);
	r.resize(basis_size);
	c_beta.resize(basis_size);
	a_beta_r.resize(basis_size);

	char variable_type;
	for (unsigned int j = 0, k = 0; j < num_vars; j++) {
		cind[j] = j;

		// If the variable type is not continuous, make it continuous.
		grb_error = GRBgetcharattrelement(model, "VType", j, &variable_type);
		if (variable_type != 'C') {
			grb_error = GRBsetcharattrelement(model, "VType", j, GRB_CONTINUOUS);
			int_var_ids[k++] = j;
		}
	}

	unsigned int num_cuts = 0;
	while (true) {
    std::cout << num_cuts << std::endl;

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
		//int cut_var_index = *frac_var_ids.begin();
    //int cut_var_index = get_most_fractional(frac_var_ids);
    //int cut_var_index = get_least_fractional(frac_var_ids);
    int cut_var_index = get_random_var(frac_var_ids, rng);
		// Get the basis inverse.
		// TODO: Is there a faster way to get the basis inverse?
		for (unsigned int j = 0; j < basis_size; j++) {
			cval[j] = 0.0;
		}

		// Populate the basis matrix and c_beta.
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
		// Get the basis inverse.
		Binv = B.inverse();
		num_cuts += AddPureCut(cut_var_index);
		//num_cuts += AddMixedCut(cut_var_index);
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

	free(cind);
	free(cval);
	free(int_var_ids);
	free(int_var_vals);
}


int Gomory::get_random_var(const std::unordered_set<unsigned int>& frac_var_ids,
  std::mt19937 &rng) {
  std::uniform_int_distribution<int> uni(0,frac_var_ids.size()-1); // guaranteed unbiased
  int index = uni(rng);
  auto it = frac_var_ids.begin();
  if(index != 0) {
    advance(it,index-1);
  }
  return *it;
}

int Gomory::get_least_fractional(const std::unordered_set<unsigned int>& frac_var_ids) {
	double least_diff = std::numeric_limits<double>::max();
  std::unordered_set<unsigned int>::const_iterator least_var_index = frac_var_ids.begin();
  for(std::unordered_set<unsigned int>::const_iterator i = ++frac_var_ids.begin();
      i != frac_var_ids.end(); ++i) {
      double value;
      grb_error = GRBgetdblattrelement(model, "X", (*i), &value);
      int closest_int = std::round(value);
      double diff = fabs(value - closest_int);
      if(diff < least_diff) {
        least_var_index = i;
      }
    }
    return *least_var_index;
}


int Gomory::get_most_fractional(
  const std::unordered_set<unsigned int>& frac_var_ids) {
  double most_diff = 0;
  std::unordered_set<unsigned int>::const_iterator most_var_index = frac_var_ids.begin();
  for(std::unordered_set<unsigned int>::const_iterator i = ++frac_var_ids.begin();
      i != frac_var_ids.end(); ++i) {
    double value;
    grb_error = GRBgetdblattrelement(model, "X", (*i), &value);
    int closest_int = std::round(value);
    double diff = fabs(value - closest_int);
    if(diff > most_diff) {
      most_var_index = i;
    }
  }
  return *most_var_index;
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
