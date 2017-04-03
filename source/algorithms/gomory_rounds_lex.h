#pragma once

#include "gomory_lex.h"

class GomoryRoundsLex : public GomoryLex {
public:
  GomoryRoundsLex(const rapidjson::Value& root);
  ~GomoryRoundsLex(void);
  void Run(void);
  int Step(void);

protected:
  int AddPureRounds(void);
  int AddMixedRounds(void);
};
