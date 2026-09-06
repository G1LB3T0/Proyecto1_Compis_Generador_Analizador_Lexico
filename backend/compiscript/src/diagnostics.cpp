#include "compiscript/diagnostics.h"

#include <utility>

namespace compiscript {

CollectingErrorListener::CollectingErrorListener(std::string category)
    : category_(std::move(category)) {}

void CollectingErrorListener::syntaxError(antlr4::Recognizer *, antlr4::Token *,
                                          size_t line,
                                          size_t char_position_in_line,
                                          const std::string &message,
                                          std::exception_ptr) {
  const std::string prefix = category_ == "léxico" ? "LEX" : "SYN";
  diagnostics_.push_back({prefix + "001", category_, "error", message, line,
                          char_position_in_line + 1});
}

} // namespace compiscript
