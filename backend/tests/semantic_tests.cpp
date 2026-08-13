#include "ast.h"
#include "semantic_analyzer.h"
#include "semantic_json.h"
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace {

std::shared_ptr<ASTNode> node(ASTNodeKind kind,
                              const std::string& name,
                              const std::string& type,
                              int line) {
    auto result = std::make_shared<ASTNode>();
    result->kind = kind;
    result->name = name;
    result->type_name = type;
    result->span = {line, 1, line, 2};
    return result;
}

std::map<std::string, int> diagnosticCounts(const SemanticModel& model) {
    std::map<std::string, int> result;
    for (const auto& diagnostic : model.diagnostics)
        result[diagnostic.code]++;
    return result;
}

void assertReferentialIntegrity(const SemanticModel& model) {
    for (const auto& scope : model.scopes) {
        assert(scope.id >= 0 && scope.id < static_cast<int>(model.scopes.size()));
        assert(scope.parent_id < scope.id);
        for (int symbol_id : scope.symbol_ids) {
            assert(symbol_id >= 0 &&
                   symbol_id < static_cast<int>(model.symbols.size()));
            assert(model.symbols[symbol_id].scope_id == scope.id);
        }
    }
    for (const auto& symbol : model.symbols) {
        assert(symbol.type_id >= 0 &&
               symbol.type_id < static_cast<int>(model.types.size()));
        assert(symbol.scope_id >= 0 &&
               symbol.scope_id < static_cast<int>(model.scopes.size()));
    }
    for (const auto& cls : model.classes) {
        assert(cls.type_id >= 0 && cls.type_id < static_cast<int>(model.types.size()));
        assert(cls.scope_id >= 0 && cls.scope_id < static_cast<int>(model.scopes.size()));
        assert(cls.symbol_id >= 0 && cls.symbol_id < static_cast<int>(model.symbols.size()));
    }
}

void validProgramProducesTypesScopesAndClass() {
    auto program = node(ASTNodeKind::PROGRAM, "<global>", "", 1);
    auto cls = node(ASTNodeKind::CLASS_DECL, "Cuenta", "Cuenta", 1);
    cls->children.push_back(node(ASTNodeKind::FIELD_DECL, "saldo", "int", 2));

    auto method = node(ASTNodeKind::METHOD_DECL, "retirar", "float", 3);
    method->children.push_back(
        node(ASTNodeKind::PARAMETER_DECL, "monto", "float", 3));
    auto block = node(ASTNodeKind::BLOCK_STMT, "block", "", 3);
    block->children.push_back(
        node(ASTNodeKind::VARIABLE_DECL, "autorizado", "bool", 4));
    auto nested = node(ASTNodeKind::BLOCK_STMT, "block", "", 5);
    nested->children.push_back(
        node(ASTNodeKind::VARIABLE_DECL, "temporal", "int", 6));
    block->children.push_back(nested);
    method->children.push_back(block);
    cls->children.push_back(method);
    program->children.push_back(cls);
    program->children.push_back(
        node(ASTNodeKind::VARIABLE_DECL, "principal", "Cuenta", 9));

    SemanticAnalyzer analyzer;
    SemanticModel model = analyzer.analyze(program);

    assert(model.ok);
    assert(model.diagnostics.empty());
    assert(model.classes.size() == 1);
    assert(model.classes[0].name == "Cuenta");
    assert(model.classes[0].field_symbol_ids.size() == 1);
    assert(model.classes[0].method_symbol_ids.size() == 1);
    // global, clase, método, bloque y bloque anidado
    assert(model.scopes.size() == 5);
    assertReferentialIntegrity(model);

    const std::string json = semanticModelToJson(model);
    assert(json.find("\"types\"") != std::string::npos);
    assert(json.find("\"scopes\"") != std::string::npos);
    assert(json.find("\"classes\"") != std::string::npos);
}

void invalidProgramReportsOnlySupportedRules() {
    auto program = node(ASTNodeKind::PROGRAM, "<global>", "", 1);
    auto first = node(ASTNodeKind::CLASS_DECL, "Cuenta", "Cuenta", 1);
    first->children.push_back(node(ASTNodeKind::FIELD_DECL, "saldo", "int", 2));
    first->children.push_back(node(ASTNodeKind::FIELD_DECL, "saldo", "float", 3));
    first->children.push_back(node(ASTNodeKind::FIELD_DECL, "dato", "Misterio", 4));

    auto method = node(ASTNodeKind::METHOD_DECL, "operar", "void", 5);
    method->children.push_back(node(ASTNodeKind::PARAMETER_DECL, "x", "int", 5));
    method->children.push_back(node(ASTNodeKind::PARAMETER_DECL, "x", "int", 5));
    auto block = node(ASTNodeKind::BLOCK_STMT, "block", "", 5);
    block->children.push_back(
        node(ASTNodeKind::VARIABLE_DECL, "local", "Fantasma", 6));
    method->children.push_back(block);
    first->children.push_back(method);

    auto duplicate = node(ASTNodeKind::CLASS_DECL, "Cuenta", "Cuenta", 8);
    program->children.push_back(first);
    program->children.push_back(duplicate);

    SemanticAnalyzer analyzer;
    SemanticModel model = analyzer.analyze(program);
    const auto counts = diagnosticCounts(model);

    assert(!model.ok);
    assert(counts.at("SEM010") == 1); // clase duplicada
    assert(counts.at("SEM002") == 2); // campo y parámetro duplicados
    assert(counts.at("SEM003") == 2); // Misterio y Fantasma
    assertReferentialIntegrity(model);
}

} // namespace

int main() {
    validProgramProducesTypesScopesAndClass();
    invalidProgramReportsOnlySupportedRules();
    std::cout << "semantic_tests: OK\n";
    return 0;
}
