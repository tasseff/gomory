#pragma once

#include <common/document.h>

class Gomory {
public:
	Gomory(const rapidjson::Value& root);
	void Run(void);
};
