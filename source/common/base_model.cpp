#include "base_model.h"

BaseModel::BaseModel(const rapidjson::Value& root, std::string model_path_,
                     std::string solution_path_) {
	model_path = model_path_;
	grb_error = GRBloadenv(&env, "gurobi.log");
	solution_path = solution_path_;
}

BaseModel::~BaseModel(void) {
	GRBfreemodel(model);
	GRBfreeenv(env);
}

void BaseModel::Run(void) {
}
