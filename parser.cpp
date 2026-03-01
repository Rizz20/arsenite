#include <iostream>
#include <fstream>
#include <sstream>

#include "lexer.h"

#include "lexer.cpp"

// default types
enum Type {
    Type_u8,
    Type_u16,
    Type_u32,
    Type_u64,
    Type_i8,
    Type_i16,
    Type_i32,
    Type_i64,
    Type_f32,
    Type_f64,
    Type_char8,
    Type_char16,
    Type_char32,
    Type_string,
};

struct Parameter {
    std::string name;
    Type type;
};

struct Statement {
    int a;
};

struct FunctionDefinition {
    std::vector<Parameter> parameters;
    Type return_type;
    std::string name;
    std::vector<Statement> statements;
};

// proc main(argc: u64, argv: []string) {}

// bool parse_function_definition(Lexer& l, FunctionDefinition& dest)
// {
//     FunctionDefinition f{};
//     Token t{};
// 
//     if (!lexer_expect(l, Tok_proc))
//         return false;
// 
//     if (!lexer_expect_and_get(l, Tok_Identifier, t))
//         return false;
//     f.name = t.literal;
// 
//     if (!lexer_expect(l, Tok_LParen))
//         return false;
// 
// 
//     if (!lexer_expect(l, Tok_RParen))
//         return false;
// }

int main() {

    std::ifstream file("main.at");
    std::ostringstream ss;

    ss << file.rdbuf();

    std::string source_code = ss.str();

    Lexer l = lexer_lex_file(source_code);

    while (!lexer_is_eof(l)) {
        lexer_print_token(lexer_current(l));
        lexer_move_next(l);
    }

    return 0;
}

