#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <iostream>
#include <vector>

enum TokenKind {
    Tok_Illegal,
    Tok_EndOfFile,

    Tok_Identifier,
    Tok_Number,
    Tok_String,

    Tok_CompEqual,
    Tok_CompLessThan,
    Tok_CompGreaterThan,
    Tok_CompLessThanEqual,
    Tok_CompGreaterThanEqual,
    Tok_CompNotEqual,
    Tok_Negate,

    Tok_LParen,     // (
    Tok_RParen,     // )
    Tok_LBrace,     // {
    Tok_RBrace,     // }
    Tok_LBracket,   // [
    Tok_RBracket,   // ]
    Tok_Colon,      // :
    Tok_Comma,      // ,
    Tok_Semicolon,  // ;
    Tok_Dquote,

    Tok_Equal,      // =
    Tok_At,
    Tok_Star,
    Tok_Add,
    Tok_Sub,
    Tok_Mul, // TODO
    Tok_Div, // TODO

    // types
    Tok_u8,
    Tok_u16,
    Tok_u32,
    Tok_u64,
    Tok_i8,
    Tok_i16,
    Tok_i32,
    Tok_i64,
    Tok_f32,
    Tok_f64,
    Tok_char8,
    Tok_char16,
    Tok_char32,

    // Keywords
    Tok_proc,
    Tok_include,
    Tok_let,
    Tok_debug_print,
    Tok_struct,
    Tok_enum,
    Tok_if,
    Tok_else,
    Tok_else_if, // TODO: add else if support in lexer
    Tok_while,
    Tok_for,
    Tok_do,
};

struct Token {
    TokenKind kind;
    std::string literal;

    Token() {
          kind = Tok_Illegal;
    }

    Token(TokenKind k, const std::string& lit)
    : kind(k), literal(lit) {}
};

struct Lexer {
    std::vector<Token> tokens;
    size_t current;
};

// bool lexer_next(Lexer& l, Token& dest);
bool  lexer_is_eof(Lexer& l);
Token lexer_current(Lexer& l);
void  lexer_move_next(Lexer& l);
Lexer lexer_lex_file(const std::string& text);
void  lexer_print_token(Token t);

#endif // LEXER_H
