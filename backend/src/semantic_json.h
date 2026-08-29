#pragma once

#include "semantic_model.h"
#include <string>

std::string semanticModelToJson(const SemanticModel& model);
std::string semanticSkippedToJson(const std::string& reason);
