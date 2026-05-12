#include <vector>
#include <unordered_map>
#include <cctype>
#include <stdio.h>

#include "lexer.hpp"

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
    {"struct", Tok_struct},
    {"enum", Tok_enum},
    {"if", Tok_if},
    {"else", Tok_else},
    {"while", Tok_while},
    {"for", Tok_for},
    {"do", Tok_do},
    {"return", Tok_return},
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
    {"string", Tok_string},
};

void lexer_print_token(Token t)
{
    switch(t.kind) {
        case Tok_Illegal             : printf("Tok_Illegal: %s\n", t.literal.c_str()); break;
        case Tok_EndOfFile           : printf("Tok_EndOfFile: %s\n", t.literal.c_str()); break;
        case Tok_Identifier          : printf("Tok_Identifier: %s\n", t.literal.c_str()); break;
        case Tok_StringLit           : printf("Tok_StringLit: %s\n", t.literal.c_str()); break;
        case Tok_Number              : printf("Tok_Number: %s\n", t.literal.c_str()); break;
        case Tok_CompEqual           : printf("Tok_CompEqual: %s\n", t.literal.c_str()); break;
        case Tok_CompLessThan        : printf("Tok_CompLessThan: %s\n", t.literal.c_str()); break;
        case Tok_CompGreaterThan     : printf("Tok_CompGreaterThan: %s\n", t.literal.c_str()); break;
        case Tok_CompLessThanEqual   : printf("Tok_CompLessThanEqual: %s\n", t.literal.c_str()); break;
        case Tok_CompGreaterThanEqual: printf("Tok_CompGreaterThanEqual: %s\n", t.literal.c_str()); break;
        case Tok_CompNotEqual        : printf("Tok_CompNotEqual: %s\n", t.literal.c_str()); break;
        case Tok_Exclam              : printf("Tok_Exclam: %s\n", t.literal.c_str()); break;
        case Tok_LParen              : printf("Tok_LParen: %s\n", t.literal.c_str()); break;
        case Tok_RParen              : printf("Tok_RParen: %s\n", t.literal.c_str()); break;
        case Tok_LBrace              : printf("Tok_LBrace: %s\n", t.literal.c_str()); break;
        case Tok_RBrace              : printf("Tok_RBrace: %s\n", t.literal.c_str()); break;
        case Tok_LBracket            : printf("Tok_LBracket: %s\n", t.literal.c_str()); break;
        case Tok_RBracket            : printf("Tok_RBracket: %s\n", t.literal.c_str()); break;
        case Tok_Colon               : printf("Tok_Colon: %s\n", t.literal.c_str()); break;
        case Tok_Comma               : printf("Tok_Comma: %s\n", t.literal.c_str()); break;
        case Tok_Semicolon           : printf("Tok_Semicolon: %s\n", t.literal.c_str()); break;
        case Tok_Dquote              : printf("Tok_Dquote: %s\n", t.literal.c_str()); break;
        case Tok_Equal               : printf("Tok_Equal: %s\n", t.literal.c_str()); break;
        case Tok_At                  : printf("Tok_At: %s\n", t.literal.c_str()); break;
        case Tok_Star                : printf("Tok_Star: %s\n", t.literal.c_str()); break;
        case Tok_Plus                : printf("Tok_Plus: %s\n", t.literal.c_str()); break;
        case Tok_Minus               : printf("Tok_Minus: %s\n", t.literal.c_str()); break;
        case Tok_FSlash              : printf("Tok_FSlash: %s\n", t.literal.c_str()); break;
        case Tok_Percentage          : printf("Tok_FSlash: %s\n", t.literal.c_str()); break;
        case Tok_u8                  : printf("Tok_u8: %s\n", t.literal.c_str()); break;
        case Tok_u16                 : printf("Tok_u16: %s\n", t.literal.c_str()); break;
        case Tok_u32                 : printf("Tok_u32: %s\n", t.literal.c_str()); break;
        case Tok_u64                 : printf("Tok_u64: %s\n", t.literal.c_str()); break;
        case Tok_i8                  : printf("Tok_i8: %s\n", t.literal.c_str()); break;
        case Tok_i16                 : printf("Tok_i16: %s\n", t.literal.c_str()); break;
        case Tok_i32                 : printf("Tok_i32: %s\n", t.literal.c_str()); break;
        case Tok_i64                 : printf("Tok_i64: %s\n", t.literal.c_str()); break;
        case Tok_f32                 : printf("Tok_f32: %s\n", t.literal.c_str()); break;
        case Tok_f64                 : printf("Tok_f64: %s\n", t.literal.c_str()); break;
        case Tok_char8               : printf("Tok_char8: %s\n", t.literal.c_str()); break;
        case Tok_char16              : printf("Tok_char16: %s\n", t.literal.c_str()); break;
        case Tok_char32              : printf("Tok_char32: %s\n", t.literal.c_str()); break;
        case Tok_string              : printf("Tok_string: %s\n", t.literal.c_str()); break;
        case Tok_proc                : printf("Tok_proc: %s\n", t.literal.c_str()); break;
        case Tok_include             : printf("Tok_include: %s\n", t.literal.c_str()); break;
        case Tok_let                 : printf("Tok_let: %s\n", t.literal.c_str()); break;
        case Tok_struct              : printf("Tok_struct: %s\n", t.literal.c_str()); break;
        case Tok_enum                : printf("Tok_enum: %s\n", t.literal.c_str()); break;
        case Tok_if                  : printf("Tok_if: %s\n", t.literal.c_str()); break;
        case Tok_else                : printf("Tok_else: %s\n", t.literal.c_str()); break;
        case Tok_else_if             : printf("Tok_else_if: %s\n", t.literal.c_str()); break;
        case Tok_while               : printf("Tok_while: %s\n", t.literal.c_str()); break;
        case Tok_for                 : printf("Tok_for: %s\n", t.literal.c_str()); break;
        case Tok_do                  : printf("Tok_do: %s\n", t.literal.c_str()); break;
        case Tok_return              : printf("Tok_return: %s\n", t.literal.c_str()); break;
    }
}

std::string lexer_tokenkind_string(TokenKind kind)
{
    switch (kind) {
    case Tok_Illegal:              return "Illegal";        break;
    case Tok_EndOfFile:            return "End of File";    break;

    case Tok_Identifier:           return "Identifier";     break;
    case Tok_Number:               return "Number Literal"; break;
    case Tok_StringLit:            return "StringLit";      break;

    case Tok_CompEqual:            return "==";             break;
    case Tok_CompLessThan:         return "<";              break;
    case Tok_CompGreaterThan:      return ">";              break;
    case Tok_CompLessThanEqual:    return "<=";             break;
    case Tok_CompGreaterThanEqual: return ">=";             break;
    case Tok_CompNotEqual:         return "!=";             break;
    case Tok_Exclam:               return "!";              break;

    case Tok_LParen:               return "(" ;             break;
    case Tok_RParen:               return ")" ;             break;
    case Tok_LBrace:               return "{" ;             break;
    case Tok_RBrace:               return "}" ;             break;
    case Tok_LBracket:             return "[" ;             break;
    case Tok_RBracket:             return "]" ;             break;
    case Tok_Colon:                return ":" ;             break;
    case Tok_Comma:                return "," ;             break;
    case Tok_Semicolon:            return ";" ;             break;
    case Tok_Dquote:               return "\"";             break;

    case Tok_Equal:                return "=" ;             break;
    case Tok_At:                   return "@";              break;
    case Tok_Star:                 return "*";              break;
    case Tok_Plus:                 return "+";              break;
    case Tok_Minus:                return "-";              break;
    case Tok_FSlash:               return "/";              break;
    case Tok_Percentage:           return "%";              break;

    // types
    case Tok_u8:                   return "u8";             break;
    case Tok_u16:                  return "u16";            break;
    case Tok_u32:                  return "u32";            break;
    case Tok_u64:                  return "u64";            break;
    case Tok_i8:                   return "i8";             break;
    case Tok_i16:                  return "i16";            break;
    case Tok_i32:                  return "i32";            break;
    case Tok_i64:                  return "i64";            break;
    case Tok_f32:                  return "f32";            break;
    case Tok_f64:                  return "f64";            break;
    case Tok_char8:                return "char8";          break;
    case Tok_char16:               return "char16";         break;
    case Tok_char32:               return "char32";         break;
    case Tok_string:               return "string";         break;

    // Keywords
    case Tok_proc:                 return "proc";           break;
    case Tok_include:              return "include";        break;
    case Tok_let:                  return "let";            break;
    case Tok_struct:               return "struct";         break;
    case Tok_enum:                 return "enum";           break;
    case Tok_if:                   return "if";             break;
    case Tok_else:                 return "else";           break;
    case Tok_else_if:              return "else if" ;       break;
    case Tok_while:                return "while";          break;
    case Tok_for:                  return "for";            break;
    case Tok_do:                   return "do";             break;
    case Tok_return:               return "return";             break;
    }

    return "";
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

        } else if (isDigit(lookahead)) { //else if for number checking
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

        } else if (lookahead == '"') {
            i++;
            std::string s = "";
            while (i < n && text[i] != '"') {
                if (text[i] == '\\' && i + 1 < n) {
                    i++;
                    if (text[i] == 'n') s += '\n';
                    else if (text[i] == 't') s += '\t';
                    else if (text[i] == '"') s += '"';
                    else if (text[i] == '\\') s += '\\';
                    else s += text[i];
                } else {
                    s += text[i];
                }
                i++;
            }
            if (i < n) i++; // skip closing quote
            tokens.push_back(Token(Tok_StringLit, s));
            continue;
        } else {
            if (lookahead == '=' && i+1< n && text[i+1] == '=') {
                tokens.push_back(Token(Tok_CompEqual, "=="));
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
                case '>': tokens.push_back(Token(Tok_CompGreaterThan, ">")); break;
                case '<': tokens.push_back(Token(Tok_CompLessThan, "<")); break;
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
