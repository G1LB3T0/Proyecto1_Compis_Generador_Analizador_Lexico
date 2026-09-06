// Recorre el parse tree de ANTLR y conserva su jerarquía en formato JSON.
#include "compiscript/tree_serializer.h"

#include "compiscript/json.h"
#include "compiscript/token_classifier.h"

#include <sstream>

namespace compiscript {

namespace {

std::string serialize(antlr4::tree::ParseTree *node, antlr4::Parser &parser) {
  std::ostringstream output;
  // Las hojas son tokens reales; conservan lexema, categoría y posición.
  if (auto *terminal = dynamic_cast<antlr4::tree::TerminalNode *>(node)) {
    antlr4::Token *token = terminal->getSymbol();
    std::string name(parser.getVocabulary().getSymbolicName(token->getType()));
    if (name.empty()) {
      name =
          std::string(parser.getVocabulary().getLiteralName(token->getType()));
    }
    if (name.empty())
      name = std::to_string(token->getType());
    const std::string text = token->getText();
    output << "{\"kind\":\"token\",\"name\":" << quoteJson(name)
           << ",\"category\":" << quoteJson(classifyToken(name, text))
           << ",\"text\":" << quoteJson(text)
           << ",\"line\":" << token->getLine()
           << ",\"column\":" << token->getCharPositionInLine() + 1
           << ",\"children\":[]}";
    return output.str();
  }

  // Los demás nodos representan reglas y se serializan recursivamente.
  auto *context = dynamic_cast<antlr4::ParserRuleContext *>(node);
  std::string name = "unknown";
  size_t line = 0;
  size_t column = 0;
  if (context) {
    const size_t rule = context->getRuleIndex();
    if (rule < parser.getRuleNames().size())
      name = parser.getRuleNames()[rule];
    if (context->getStart()) {
      line = context->getStart()->getLine();
      column = context->getStart()->getCharPositionInLine() + 1;
    }
  }
  output << "{\"kind\":\"rule\",\"name\":" << quoteJson(name)
         << ",\"text\":\"\",\"line\":" << line << ",\"column\":" << column
         << ",\"children\":[";
  for (size_t index = 0; index < node->children.size(); ++index) {
    if (index)
      output << ',';
    output << serialize(node->children[index], parser);
  }
  output << "]}";
  return output.str();
}

} // namespace

std::string parseTreeToJson(antlr4::tree::ParseTree *tree,
                            antlr4::Parser &parser) {
  return serialize(tree, parser);
}

} // namespace compiscript
