#include <iostream>
#include <common/document.h>
#include <common/error.h>
#include <common/file.h>
#include "gomory.h"

Gomory::Gomory(const rapidjson::Value& root) {
	std::cout << "Constructing Gomory." << std::endl;
}

void Gomory::Run(void) {
	std::cout << "Running Gomory." << std::endl;
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
