#pragma once

#include "compiscript/model.h"

namespace compiscript {

// Tercera pasada: cuádruplos estructurados -> ensamblador MIPS32 para MARS.
class MipsGenerator {
public:
  MipsGenerator(const SemanticResult &semantic, const TacResult &tac)
      : semantic_(semantic), tac_(tac) {}
  MipsResult generate();

private:
  const SemanticResult &semantic_;
  const TacResult &tac_;
};

} // namespace compiscript
