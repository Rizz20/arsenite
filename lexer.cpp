#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <cctype>
#include <utility>
#include <stdio.h>

#include "lexer.h"

bool isLetter(char c) {
    return (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z');
}

bool isDigit(char c) {
    return (c >= '0' && c <= '9');
}

bool isIdentifierStart(char c) {
    return isLetter(c) || c == '_';
}

bool isIdentifierChar(char c) {
    return  isIdentifierStart(c) || isDigit(c);
}

std::unordered_map<std::string, TokenKind> keywordTable = {
    {"proc", Tok_proc},
    {"include", Tok_include},
    {"let", Tok_let},
    {"debug_print", Tok_debug_print},
    {"struct", Tok_struct},
    {"enum", Tok_enum},
    {"if", Tok_if},
    {"else", Tok_else},
    {"while", Tok_while},
    {"for", Tok_for},
    {"do", Tok_do},
    {"u8", Tok_u8},
    {"u16", Tok_u16},
    {"u32", Tok_u32},
    {"u64", Tok_u64},
    {"i8", Tok_i8},
    {"i16", Tok_i16},
    {"i32", Tok_i32},
    {"i64", Tok_i64},
    {"f32", Tok_f32},
    {"f64", Tok_f64},
    {"char8", Tok_char8},
    {"char16", Tok_char16},
    {"char32", Tok_char32},
};

void lexer_print_token(Token t)
{
    switch(t.kind) {
        case Tok_Illegal             : printf("Tok_Illegal\n"); break;
        case Tok_EndOfFile           : printf("Tok_EndOfFile\n"); break;
        case Tok_Identifier          : printf("Tok_Identifier\n"); break;
        case Tok_Number              : printf("Tok_Number\n"); break;
        case Tok_String              : printf("Tok_String\n"); break;
        case Tok_CompEqual           : printf("Tok_CompEqual\n"); break;
        case Tok_CompLessThan        : printf("Tok_CompLessThan\n"); break;
        case Tok_CompGreaterThan     : printf("Tok_CompGreaterThan\n"); break;
        case Tok_CompLessThanEqual   : printf("Tok_CompLessThanEqual\n"); break;
        case Tok_CompGreaterThanEqual: printf("Tok_CompGreaterThanEqual\n"); break;
        case Tok_CompNotEqual        : printf("Tok_CompNotEqual\n"); break;
        case Tok_Exclam              : printf("Tok_Exclam\n"); break;
        case Tok_LParen              : printf("Tok_LParen\n"); break;
        case Tok_RParen              : printf("Tok_RParen\n"); break;
        case Tok_LBrace              : printf("Tok_LBrace\n"); break;
        case Tok_RBrace              : printf("Tok_RBrace\n"); break;
        case Tok_LBracket            : printf("Tok_LBracket\n"); break;
        case Tok_RBracket            : printf("Tok_RBracket\n"); break;
        case Tok_Colon               : printf("Tok_Colon\n"); break;
        case Tok_Comma               : printf("Tok_Comma\n"); break;
        case Tok_Semicolon           : printf("Tok_Semicolon\n"); break;
        case Tok_Dquote              : printf("Tok_Dquote\n"); break;
        case Tok_Equal               : printf("Tok_Equal\n"); break;
        case Tok_At                  : printf("Tok_At\n"); break;
        case Tok_Star                : printf("Tok_Star\n"); break;
        case Tok_Plus                : printf("Tok_Plus\n"); break;
        case Tok_Minus               : printf("Tok_Minus\n"); break;
        case Tok_FSlash              : printf("Tok_FSlash\n"); break;
        case Tok_Percentage          : printf("Tok_FSlash\n"); break;
        case Tok_Div                 : printf("Tok_Div\n"); break;
        case Tok_u8                  : printf("Tok_u8\n"); break;
        case Tok_u16                 : printf("Tok_u16\n"); break;
        case Tok_u32                 : printf("Tok_u32\n"); break;
        case Tok_u64                 : printf("Tok_u64\n"); break;
        case Tok_i8                  : printf("Tok_i8\n"); break;
        case Tok_i16                 : printf("Tok_i16\n"); break;
        case Tok_i32                 : printf("Tok_i32\n"); break;
        case Tok_i64                 : printf("Tok_i64\n"); break;
        case Tok_f32                 : printf("Tok_f32\n"); break;
        case Tok_f64                 : printf("Tok_f64\n"); break;
        case Tok_char8               : printf("Tok_char8\n"); break;
        case Tok_char16              : printf("Tok_char16\n"); break;
        case Tok_char32              : printf("Tok_char32\n"); break;
        case Tok_proc                : printf("Tok_proc\n"); break;
        case Tok_include             : printf("Tok_include\n"); break;
        case Tok_let                 : printf("Tok_let\n"); break;
        case Tok_debug_print         : printf("Tok_debug_print\n"); break;
        case Tok_struct              : printf("Tok_struct\n"); break;
        case Tok_enum                : printf("Tok_enum\n"); break;
        case Tok_if                  : printf("Tok_if\n"); break;
        case Tok_else                : printf("Tok_else\n"); break;
        case Tok_else_if             : printf("Tok_else_if\n"); break;
        case Tok_while               : printf("Tok_while\n"); break;
        case Tok_for                 : printf("Tok_for\n"); break;
        case Tok_do                  : printf("Tok_do\n"); break;
    }
}

Lexer lexer_lex_file(const std::string& text) {
    std::vector<Token> tokens;

    int i = 0;
    int n = text.length();

    while (i < n) {

        char lookahead = text[i];

        if (isspace(lookahead)){
            i++;
            continue;
        }

        if (isIdentifierStart(lookahead)) {

            std::string ident = "";

            while (i < n && isIdentifierChar(text[i])) {
                ident += text[i];
                i++;
            }

            Token tok;

            auto it = keywordTable.find(ident);

            if (it != keywordTable.end())
                tok.kind = it->second;
            else
                tok.kind = Tok_Identifier;

            tok.literal = ident;
            tokens.push_back(tok);
        }

        //else if for number checking
        else if (isDigit(lookahead)) {
            std::string number = "";
            bool seenDot = false;

            while (i < n) {
                char c = text[i];

                if (isDigit(c)) {
                    number += c;
                    i++;
                }
                else if (c == '.' && !seenDot) {
                    seenDot = true;
                    number += c;
                    i++;
                }
                else break;
            }

            tokens.push_back(Token(Tok_Number, number));
        }
        //debug print fix - double quotations
        else {
            if (lookahead=='=' && i+1<n && text[i+1]=='=') {
                tokens.push_back(Token(Tok_CompEqual,"=="));
                i+=2; continue;
                }
            if (lookahead=='<' && i+1<n && text[i+1]=='=') {
                tokens.push_back(Token(Tok_CompLessThanEqual,"<="));
                i+=2; continue;
            }
            if (lookahead=='>' && i+1<n && text[i+1]=='=') {
                tokens.push_back(Token(Tok_CompGreaterThanEqual,">="));
                i+=2; continue;
            }
            if (lookahead=='!' && i+1<n && text[i+1]=='=') {
                tokens.push_back(Token(Tok_CompNotEqual,"!="));
                i+=2; continue;
            }
            switch (lookahead) {
                case '"': tokens.push_back(Token(Tok_Dquote, "\""));break;
                case '+': tokens.push_back(Token(Tok_Plus, "+")); break;
                case '-': tokens.push_back(Token(Tok_Minus, "-")); break;
                case '(': tokens.push_back(Token(Tok_LParen,"(")); break;
                case ')': tokens.push_back(Token(Tok_RParen,")")); break;
                case '{': tokens.push_back(Token(Tok_LBrace,"{")); break;
                case '}': tokens.push_back(Token(Tok_RBrace,"}")); break;
                case '[': tokens.push_back(Token(Tok_LBracket,"[")); break;
                case ']': tokens.push_back(Token(Tok_RBracket,"]")); break;
                case ':': tokens.push_back(Token(Tok_Colon,":")); break;
                case ',': tokens.push_back(Token(Tok_Comma,",")); break;
                case ';': tokens.push_back(Token(Tok_Semicolon,";")); break;
                case '=': tokens.push_back(Token(Tok_Equal,"=")); break;
                case '@': tokens.push_back(Token(Tok_At,"@")); break;
                case '*': tokens.push_back(Token(Tok_Star,"*")); break;
                case '/': tokens.push_back(Token(Tok_FSlash,"/")); break;
                case '!': tokens.push_back(Token(Tok_Exclam,"!")); break;
                case '%': tokens.push_back(Token(Tok_Percentage, "%")); break;
                default:
                    std::string l{lookahead};
                    tokens.push_back(Token(Tok_Illegal, l));
                    break;
            }

            i++;
        }
    }

    Lexer l = {.tokens = tokens, .current = 0};
    return l;
}

bool lexer_is_eof(Lexer& l)
{
    return (l.current >= l.tokens.size() || l.tokens[l.current].kind == Tok_EndOfFile);
}

Token lexer_current(Lexer& l)
{
    Token t{};
    if (lexer_is_eof(l))
        t.kind = Tok_EndOfFile;
    else
        t = l.tokens[l.current];

    return t;
}

Token lexer_next(Lexer& l)
{
    Token t = Token(Tok_EndOfFile, "");
    l.current += 1;
    if (!lexer_is_eof(l)) {
        t = l.tokens[l.current];
    }

    return t;
}
