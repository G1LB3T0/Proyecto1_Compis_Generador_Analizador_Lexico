#pragma once
#include <string>
#include <vector>

struct ParseStep {
    std::string stack;    // LR: state stack "0 3 7" | LL1: parse stack "E' $"
    std::string symbols;  // LR: symbol stack "$ id +" | LL1: ""
    std::string input;    // remaining input types: "id + id $"
    std::string action;   // action description
};
