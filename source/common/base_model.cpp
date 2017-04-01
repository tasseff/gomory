#include "base_model.h"

BaseModel::BaseModel(const rapidjson::Value& root) {
	std::string model_path = root["model"].GetString();
	grb_error = GRBloadenv(&env, "gurobi.log");
	solution_path = root["solution"].GetString();
}

BaseModel::~BaseModel(void) {
	GRBfreemodel(model);
	GRBfreeenv(env);
}

void BaseModel::Run(void) {
}
