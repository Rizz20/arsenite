#include <iostream>
#include <fstream>
#include <sstream>
#include "parser.hpp"

// Simple helper to map our types to QBE types
// w = 32-bit, l = 64-bit
std::string map_qbe_type(uint64_t type_id) {
    switch (type_id) {
        case DefaultType_u64: return "l";
        case DefaultType_u32: return "w";
        default: return "w";
    }
}

void emit_qbe(const FunctionDefinition& f) {
    // 1. Function Header
    // QBE uses $ for global symbols and 'w' or 'l' for return types
    std::string ret_t = f.return_type ? map_qbe_type(f.return_type->type_id) : "w";
    printf("export function %s $%s(", ret_t.c_str(), f.name.c_str());
    
    // Handle parameters
    for (size_t i = 0; i < f.parameters.size(); ++i) {
        printf("%s %%%s%s", 
               map_qbe_type(f.parameters[i].type.type_id).c_str(),
               f.parameters[i].name.c_str(),
               (i == f.parameters.size() - 1) ? "" : ", ");
    }
    printf(") {\n@start\n");

    // 2. Iterate Statements
    for (auto* stmt : f.statements) {
        if (stmt->kind == Statement_Definition) {
            auto* def = stmt->definition_statement;
            // For now, we assume simple constants or copies
            if (def->right && def->right->kind == Expr_Atom) {
                printf("    %%%s =%s copy %s\n", 
                       def->name.c_str(), 
                       map_qbe_type(def->type.type_id).c_str(),
                       def->right->at.value.c_str());
            }
        } else if (stmt->kind == Statement_Return) {
            auto* ret = stmt->return_statement;
            if (ret->root && ret->root->kind == Expr_Atom) {
                printf("    ret %s\n", ret->root->at.value.c_str());
            }
        }
    }

    printf("}\n");
}

int main() {
    // Read the source file
    std::ifstream file("main.at");
    if (!file.is_open()) return 1;
    
    std::ostringstream ss;
    ss << file.rdbuf();
    
    // Lex and Parse
    Lexer l = lexer_lex_file(ss.str());
    FunctionDefinition f{};
    
    if (parse_function_definition(l, f)) {
        emit_qbe(f);
    } else {
        std::cerr << "Codegen failed: Parsing error." << std::endl;
        return 1;
    }

    return 0;
}
