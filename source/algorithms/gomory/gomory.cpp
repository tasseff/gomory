#include <iostream>
#include <iterator>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_set>
#include <common/document.h>
#include <common/error.h>
#include <common/file.h>
#include "gomory.h"

Gomory::Gomory(const rapidjson::Value& root) {
	std::string model_path = root["model"].GetString();
	epsilon = root["epsilon"].GetDouble();

	env = new GRBEnv();

	if (root["gurobiPresolve"].GetBool() == false)
		env->set(GRB_IntParam_Presolve, 0);
	if (root["gurobiCuts"].GetBool() == false)
		env->set(GRB_IntParam_Cuts, 0);
	if (root["gurobiBB"].GetBool() == false)
		env->set(GRB_DoubleParam_NodeLimit, 1.0);
	if (root["gurobiHeuristics"].GetBool() == false)
		env->set(GRB_DoubleParam_Heuristics, 0.0);

	model = new GRBModel(*env, model_path);
}

Gomory::~Gomory(void) {
	delete model;
	delete env;
}

void Gomory::Run(void) {
	// Keep track of the variables that were originally integer.
	std::unordered_set<unsigned int> int_var_ids;

	// Convert integer variables to continuous variables.
	GRBVar* vars = model->getVars();

	std::unordered_set<unsigned int> int_vars;
	for (int j = 0; j < model->get(GRB_IntAttr_NumVars); j++) {
		if (vars[j].get(GRB_CharAttr_VType) != GRB_CONTINUOUS) {
			vars[j].set(GRB_CharAttr_VType, GRB_CONTINUOUS);
			int_var_ids.insert(j);
		}
	}

	// Initialize an ordered set to keep track of fractional variables.
	std::unordered_set<unsigned int> frac_var_ids;
	std::unordered_set<unsigned int>::iterator it;

	// While there are variables with fractional values, perform the algorithm.
	while (true) {
		// Write the algorithm here...
		model->optimize();

		// Update the list of fractional variables.
		for (it = int_var_ids.begin(); it != int_var_ids.end(); ++it) {
			double tmp = vars[*it].get(GRB_DoubleAttr_X);

			// Add the variable to the set if it's fractional.
			if (fabs(tmp - floor(tmp + 0.5)) > epsilon) {
				frac_var_ids.insert(*it); // TODO: Change to emplace when using gcc >= 4.8.
			} else {
				frac_var_ids.erase(*it); // Is this valid if the value isn't in the set?
			}
		}

		// Remove this when the algorithm is finished...
		// e.g., if (frac_var_ids.size() == 0)
		break;
	}

	int optimstatus = model->get(GRB_IntAttr_Status);
	if (optimstatus == GRB_OPTIMAL) {
		double objval = model->get(GRB_DoubleAttr_ObjVal);
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
