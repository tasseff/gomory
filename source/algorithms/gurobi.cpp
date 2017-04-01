#include <iostream>
#include <common/document.h>
#include <common/error.h>
#include "gurobi.h"

Gurobi::Gurobi(const rapidjson::Value& root) : BaseModel(root) {
	std::string model_path = root["model"].GetString();
	grb_error = GRBreadmodel(env, model_path.c_str(), &model);
}

void Gurobi::Run(void) {
	grb_error = GRBoptimize(model);
	grb_error = GRBwrite(model, solution_path.c_str());
}

int main(int argc, char* argv[]) {
	// Check if a JSON file has been specified.
	if (argc <= 1) {
		PrintErrorAndExit("JSON file has not been specified.");
	}

	// Read the scenario file.
	File model_file(argv[1]);

	// Parse the scenario file as a JSON document.
	Document json(model_file);

	// Set up the model.
	Gurobi model(json.root["parameters"]);

	// Run the model.
	model.Run();

	// Program executed successfully.
	return 0;
}
