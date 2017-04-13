#include <common/document.h>
#include <common/error.h>
#include "gomory.h"

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
	Gomory model(json.root["parameters"], argv[2], argv[3]);

	// Run the model.
	model.Run();

	// Program executed successfully.
	return 0;
}
