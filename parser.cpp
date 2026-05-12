#include <cstdio>
#include <iostream>
#include <optional>
#include <string>
#include <cstdio>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include "lexer.hpp"

enum OpAss {
    OpAss_Left,
    OpAss_Right,
};

enum Operator {
    Op_Add,
    Op_Sub,
    Op_Mul,
    Op_Div,
    Op_Mod,
    Op_Assign,
};

std::unordered_map<Operator, std::string> op_string {
    {Op_Add, "+"},
    {Op_Sub, "-"},
    {Op_Mul, "*"},
    {Op_Div, "/"},
    {Op_Mod, "%"},
    {Op_Assign, "="},
};

std::unordered_map<Operator, int> precedence_table = {
    {Op_Assign,0},
    {Op_Add,   1},
    {Op_Sub,   1},
    {Op_Mul,   2},
    {Op_Div,   2},
    {Op_Mod,   3},
};

std::unordered_map<Operator, OpAss> opass_table = {
    {Op_Add, OpAss_Left},
    {Op_Sub, OpAss_Left},
    {Op_Mul, OpAss_Left},
    {Op_Div, OpAss_Left},
    {Op_Mod, OpAss_Left},
    {Op_Assign, OpAss_Right}
};

std::unordered_map<TokenKind, Operator> op_table = {
    {Tok_Plus,       Op_Add},
    {Tok_Minus,      Op_Sub},
    {Tok_Star,       Op_Mul},
    {Tok_FSlash,     Op_Div},
    {Tok_Percentage, Op_Mod},
    {Tok_Equal,      Op_Assign},
};

enum ModifierKind {
    Modifier_Arr,
    Modifier_Slice,
};

struct Modifier {
    ModifierKind kind;
    uint64_t size; // in case of Modifier_Arr, size is known at compile time
                   // in case of Modifier_Slice, size is know at runtime
};

struct Type {
    uint64_t type_id;
    std::vector<Modifier> mods;
};

enum DefaultTypes {
    DefaultType_Unknown = 0,
    DefaultType_u8,
    DefaultType_u16,
    DefaultType_u32,
    DefaultType_u64,
    DefaultType_i8,
    DefaultType_i16,
    DefaultType_i32,
    DefaultType_i64,
    DefaultType_f32,
    DefaultType_f64,
    DefaultType_char8,
    DefaultType_char16,
    DefaultType_char32,
    DefaultType_string,
    DefaultTypeCount,
};

enum AtomKind {
    Atom_Variable,
    Atom_Constant,
};

struct Atom {
    AtomKind kind;
    std::string value;
    Type type;
};

struct Expr;

struct FuncCall {
    std::string name;
    std::vector<Expr*> args;
};

struct Expr;

struct ArrIndex {
    std::string name;
    std::vector<Expr*> index;
};

enum ExprKind {
    Expr_Atom,
    Expr_Operator,
    Expr_FuncCall,
    Expr_ArrIndex,
};

struct Expr {
    ExprKind kind;
    union {
        Atom at;            // Expr_Atom
        struct {            // Expr_Operator
            Operator op;
            Expr *left;
            Expr *middle;   // use in case of ternery operators
            Expr *right;
        };
        FuncCall func_call; // Expr_FuncCall
        ArrIndex arr_index; //Expr for array indexing
    };

    Expr (ArrIndex ar)
            :kind(Expr_ArrIndex), arr_index(ar){}

    Expr (FuncCall fn)
            : kind(Expr_FuncCall), func_call(fn){}

    Expr (Atom at)
          : kind(Expr_Atom), at(at) {}

    Expr (Expr *left, Expr *right, Operator op)
          : kind(Expr_Operator), op(op), left(left), right(right) {}
};

struct Statement;
struct DeclarationStatement;
struct DefinitionStatement;
struct ExpressionStatement;
struct IfStatement;
struct ReturnStatement;

struct DeclarationStatement {
    std::string name;
    Type type;
};

struct DefinitionStatement {
    std::string name;
    Type type;
    Expr* right;
};

struct ExpressionStatement {
    Expr *root;
};

struct ReturnStatement {
    Expr *root;
};

struct IfStatement {
    Expr* if_condition;
    std::vector<Statement*> if_block_statements;

    std::vector<Expr *> else_if_conditions;
    std::vector<std::vector<Statement*>> else_if_condition_statements;

    std::vector<Statement*> else_statements;
};

enum StatementKind {
    Statement_Empty,
    Statement_Declaration,
    Statement_Definition,
    Statement_Expression,
    Statement_If,
    Statement_Return,
};

struct Statement {
    StatementKind kind;
    union {
        DeclarationStatement *declaration_statement;      // Statement_Declaration
        DefinitionStatement  *definition_statement;       // Statement_Definition
        ExpressionStatement  *expression_statement;       // Statement_Expression
        IfStatement          *if_else_statement;          // Statement_If
        ReturnStatement      *return_statement;           // Statement_Return
    };
};

struct Parameter {
    std::string name;
    Type type;
};

struct FunctionDefinition {
    std::string name;
    std::optional<Type> return_type;
    std::vector<Parameter> parameters;
    std::vector<Statement*> statements;
};

Expr* parse_primary(Lexer& l);

bool is_op(Token t){
    auto it = op_table.find(t.kind);

    return !(it == op_table.end());
}

Expr* parse_expression(Lexer& l, int min_prec) {
    Expr* primary_lhs = parse_primary(l);
    while(true){
        if (!is_op(lexer_current(l))){
            break;
        }
        Operator op = op_table[lexer_current(l).kind];
        if (precedence_table[op]<min_prec){
            break;
        }
        lexer_next(l);

        int next_min_prec = 0;

        switch (opass_table[op]) {
            case OpAss_Left:
                next_min_prec = precedence_table[op]+1; break;
            case OpAss_Right:
                next_min_prec = precedence_table[op]; break;
        }

        Expr* primary_rhs = parse_expression(l, next_min_prec);

        primary_lhs = new Expr(primary_lhs, primary_rhs, op);


    }

    return primary_lhs;

}

bool lexer_expect_and_print(Lexer& l, TokenKind kind, bool print_err)
{
    if (lexer_current(l).kind != kind) {
        if (print_err)
            printf("Exptected \"%s\", got \"%s\"\n", lexer_tokenkind_string(kind).c_str(), lexer_current(l).literal.c_str());
        return false;
    }

    lexer_next(l);
    return true;
}

bool lexer_expect(Lexer &l, TokenKind kind)
{
    return lexer_expect_and_print(l, kind, true);
}

bool lexer_expects(Lexer& l, std::vector<TokenKind> kinds)
{
    for (TokenKind kind : kinds)
        if (lexer_expect_and_print(l, kind, false))
            return true;

    return false;
}

bool parse_type_declaration(Lexer& l, Type& t)
{
    if (lexer_current(l).kind == Tok_LBracket) {
        while (true) {
            Modifier mod{};
            if (!lexer_expect(l, Tok_LBracket)) return false;
            if (lexer_current(l).kind == Tok_Number) {
                mod.kind = Modifier_Arr;
                mod.size = atoll(lexer_current(l).literal.c_str()); // TODO: replace this with custom integer parser
                lexer_next(l);
            } else {
                mod.kind = Modifier_Slice;
            }

            if (!lexer_expect(l, Tok_RBracket)) return false;

            t.mods.push_back(mod);

            if (lexer_current(l).kind != Tok_LBracket) break;
        }
    }

    TokenKind kind = lexer_current(l).kind;
    switch (kind) {
    case Tok_u8:     t.type_id = DefaultType_u8;     break;
    case Tok_u16:    t.type_id = DefaultType_u16;    break;
    case Tok_u32:    t.type_id = DefaultType_u32;    break;
    case Tok_u64:    t.type_id = DefaultType_u64;    break;
    case Tok_i8:     t.type_id = DefaultType_i8;     break;
    case Tok_i16:    t.type_id = DefaultType_i16;    break;
    case Tok_i32:    t.type_id = DefaultType_i32;    break;
    case Tok_i64:    t.type_id = DefaultType_i64;    break;
    case Tok_f32:    t.type_id = DefaultType_f32;    break;
    case Tok_f64:    t.type_id = DefaultType_f64;    break;
    case Tok_char8:  t.type_id = DefaultType_char8;  break;
    case Tok_char16: t.type_id = DefaultType_char16; break;
    case Tok_char32: t.type_id = DefaultType_char32; break;
    case Tok_string: t.type_id = DefaultType_string; break;
    default: return false;
    }

    lexer_next(l);

    return true;
}

bool parse_parameters(Lexer& l, std::vector<Parameter>& parameters)
{

    while (true) {
        Token identifier = lexer_current(l);
        if (!lexer_expect(l, Tok_Identifier)) return false;
        if (!lexer_expect(l, Tok_Colon)) return false;

        Type t;
        if (!parse_type_declaration(l, t)) return false;

        Parameter p{.name = identifier.literal, .type = t};
        parameters.push_back(p);

        if (lexer_current(l).kind != Tok_Comma) break;

        lexer_next(l);
    }

    return true;
}

bool parse_statement(Lexer& l, Statement *s)
{
    switch (lexer_current(l).kind) {
        case Tok_Semicolon:
            s->kind = Statement_Empty;
            break;
        case Tok_let: {
            if (!lexer_expect(l, Tok_let)) return false;

            std::string name = lexer_current(l).literal;
            if (!lexer_expect(l, Tok_Identifier)) return false;
            if (!lexer_expect(l, Tok_Colon)) return false;

            Type t;
            if (!parse_type_declaration(l, t)) return false;

            if (lexer_current(l).kind == Tok_Equal) {
                lexer_next(l);

                DefinitionStatement *stmt = new DefinitionStatement();
                stmt->name = name;
                stmt->right = parse_expression(l, 0);
                stmt->type = t;

                s->kind = Statement_Definition;
                s->definition_statement = stmt;
            } else {
                DeclarationStatement *stmt = new DeclarationStatement();
                stmt->name = name;
                stmt->type = t;

                s->kind = Statement_Declaration;
                s->declaration_statement = stmt;
            }

            if (!lexer_expect(l, Tok_Semicolon)) return false;

            break;
        }
        case Tok_Identifier: {
            Expr *root = parse_expression(l, 0);
            ExpressionStatement *expr_stmt = new ExpressionStatement();
            expr_stmt->root = root;

            // s = Statement(expr_stmt);
            s->kind = Statement_Expression;
            s->expression_statement = expr_stmt;

            if (!lexer_expect(l, Tok_Semicolon)) return false;
            break;
        }
        case Tok_return: {
            if (!lexer_expect(l, Tok_return)) return false;

            Expr *root = parse_expression(l, 0);
            ReturnStatement *return_stmt = new ReturnStatement();
            return_stmt->root = root;

            // s = Statement(return_stmt);
            s->kind = Statement_Return;
            s->return_statement = return_stmt;

            if (!lexer_expect(l, Tok_Semicolon)) return false;
            break;
        }
        case Tok_if: {
            break;
        }
        default:
            return false;
    }

    return true;
}

bool parse_block(Lexer& l, std::vector<Statement*>& statements)
{
    if (!lexer_expect(l, Tok_LBrace)) return false;

    while (true) {
        Statement *s = new Statement();
        if (!parse_statement(l, s)) break;
        statements.push_back(s);
    }

    if (!lexer_expect(l, Tok_RBrace)) return false;

    return true;
}

bool parse_function_definition(Lexer& l, FunctionDefinition& f)
{
    if (!lexer_expect(l, Tok_proc)) return false;

    f.name = lexer_current(l).literal;
    if (!lexer_expect(l, Tok_Identifier)) return false;
    if (!lexer_expect(l, Tok_LParen)) return false;

    if (lexer_current(l).kind == Tok_Identifier)
        if (!parse_parameters(l, f.parameters)) return false;

    if (!lexer_expect(l, Tok_RParen)) return false;

    // TODO: return type parsing
    if (lexer_current(l).kind == Tok_Minus) {
        lexer_next(l);
        Type t{};
        if (!lexer_expect(l, Tok_CompGreaterThan)) return false;
        if (!parse_type_declaration(l, t)) return false;
        f.return_type = t;
    }

    if (!parse_block(l, f.statements)) return false;

    return true;
}

Expr* parse_function_call(Lexer& l, std::string val);
Expr* parse_arr_index(Lexer& l, std::string name, std::vector<Expr*>& indexes);

Expr* parse_primary(Lexer& l){
    Expr *e = nullptr;
    switch (lexer_current(l).kind) {

        case Tok_Minus:
        {
            lexer_next(l);

            Expr* Operand = parse_expression(l, 999);

            FuncCall fn;
            fn.name = "_minus";
            fn.args.push_back(Operand);


            e = new Expr(fn);

            break;

        }
        case Tok_Star:
        {
            lexer_next(l);

            Expr* Operand = parse_expression(l, 999);

            FuncCall fn;
            fn.name = "_pointer";
            fn.args.push_back(Operand);


            e = new Expr(fn);

            break;
        }
        case Tok_Exclam:
        {
            lexer_next(l);

            Expr* Operand = parse_expression(l, 999);

            FuncCall fn;
            fn.name = "_negate";
            fn.args.push_back(Operand);


            e = new Expr(fn);

            break;

        }

        case Tok_At:
        {
            lexer_next(l);

            Expr* Operand = parse_expression(l, 999);

            FuncCall fn;
            fn.name = "_at";
            fn.args.push_back(Operand);


            e = new Expr(fn);

            break;
        }

        case Tok_Identifier:
        {
            Token t = lexer_current(l);
            Token p = lexer_next(l);

            if(p.kind == Tok_LParen){
                e = parse_function_call(l, t.literal);
                break;
            }
            else if (p.kind == Tok_LBracket) {

                std::vector<Expr*> ep;
                e = parse_arr_index(l,t.literal,ep);
                break;
            }
            else{

                Atom a {.kind = Atom_Variable, .value = t.literal, .type {.type_id = DefaultType_Unknown}};
                e = new Expr(a);
                break;
            }

        }
        case Tok_StringLit:
        {
            Atom a {.kind = Atom_Constant, .value = lexer_current(l).literal, .type {.type_id = DefaultType_string, .mods = {}}};
            e = new Expr(a);
            lexer_next(l);
            break;
        }
        case Tok_Number:
        {
            //To do later
            Atom a {.kind = Atom_Constant, .value = lexer_current(l).literal, .type {.type_id = DefaultType_Unknown}};
            e = new Expr(a);
            lexer_next(l);
            break;

        }
        case Tok_LParen:
            lexer_next(l);
            e = parse_expression(l, 0);
            if(lexer_current(l).kind != Tok_RParen){
                std::cout<<"bleh bluh bluh"<<std::endl;
            }
            lexer_next(l);
            return e;

            break;
        default:
            std::cout << "Bleh Bleh Bleh Bluh Bluh Bluh\n";
            lexer_next(l);
    }

    return e;
}

Expr* parse_arr_index(Lexer& l, std::string name, std::vector<Expr*>& indexes){
    Token p = lexer_current(l);
    if(p.kind != Tok_LBracket){
        std::cout<<"no L brace give"<<std::endl;
    }
    lexer_next(l);

    if(lexer_current(l).kind == Tok_RBracket)
        std::cout<<"Expected expression inside the thing"<<std::endl;
    Expr* e = nullptr;

    e = parse_expression(l, 0);


    if(lexer_current(l).kind != Tok_RBracket){
        std::cout<<"Plugh plugh expected ] "<<std::endl;
    }

    indexes.push_back(e);

    if(lexer_next(l).kind == Tok_LBracket){
        parse_arr_index(l,name, indexes);
    }

    ArrIndex ar = ArrIndex{.name = name, .index = indexes};

    Expr* arr = new Expr(ar);

    return arr;
}

Expr* parse_function_call(Lexer& l, std::string val)
{
    Token p = lexer_current(l);
    if(p.kind != Tok_LParen){
        std::cout<<"fah FAH"<<std::endl;
    }
    lexer_next(l);

    std::vector<Expr*> argss;

    if(lexer_current(l).kind != Tok_RParen){

        while(true){
            argss.push_back(parse_expression(l,0));

            if(lexer_current(l).kind == Tok_Comma){
                lexer_next(l);
            }else{
                break;
            }

        }

    }

    Token close = lexer_current(l);
    if(close.kind != Tok_RParen){
        printf("Expected rparen" );
    }

    FuncCall fn = FuncCall{.name = val, .args = argss};

    Expr* ep = new Expr(fn);

    lexer_next(l);

    return ep;
}

void pretty_print_expr(Expr *root, const std::string& prefix, const std::string& prefix_to_pass)
{
    if (root == nullptr) {
        std::printf("%s<null>\n", prefix.c_str());
        return;
    }

    std::printf("%s", prefix.c_str());
    switch (root->kind) {
        case Expr_Atom:
            std::printf("Atom: %s\n", root->at.value.c_str());
            break;
        case Expr_Operator:
            std::printf("Operator: %s\n", op_string[root->op].c_str());
            pretty_print_expr(root->left, prefix_to_pass + "├──╴", prefix_to_pass + "│   ");
            pretty_print_expr(root->right, prefix_to_pass + "└──╴", prefix_to_pass + "    ");
            // pretty_print_expr(root->left, prefix_to_pass + "|-- ", prefix_to_pass + "|   ");
            // pretty_print_expr(root->right, prefix_to_pass + "\\-- ", prefix_to_pass + "    ");
            break;
        case Expr_FuncCall:
            std::printf("FuncCall: %s\n", root->func_call.name.c_str());
            for (long unsigned int i = 0; i < root->func_call.args.size(); i++) {
                bool is_last = (i == root->func_call.args.size() - 1);
                pretty_print_expr(root->func_call.args[i], prefix_to_pass + (is_last ? "└──╴" : "├──╴"), prefix_to_pass + (is_last ? "    " : "│   "));
            }
            break;
            break;
        case Expr_ArrIndex:
            std::printf("ArrIndex: %s\n", root->arr_index.name.c_str());
            for (long unsigned int i = 0; i < root->arr_index.index.size(); i++) {
                bool is_last = (i == root->arr_index.index.size() - 1);
                pretty_print_expr(root->arr_index.index[i], prefix_to_pass + (is_last ? "└──╴" : "├──╴"), prefix_to_pass + (is_last ? "    " : "│   ")
                );
            }
            break;
    }
}
