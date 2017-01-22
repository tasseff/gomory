#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <common/document.h>
#include <common/error.h>
#include <common/file.h>
#include "gomory.h"

Gomory::Gomory(const rapidjson::Value& root) {
	std::string model_path = root["model"].GetString();
	env = new GRBEnv();
	model = new GRBModel(*env, model_path);
}

Gomory::~Gomory(void) {
	delete model;
	delete env;
}

void Gomory::Run(void) {
	model->optimize();
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
