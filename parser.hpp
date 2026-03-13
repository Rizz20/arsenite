#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// --- Enums ---
enum OpAss { OpAss_Left, OpAss_Right };
enum Operator { Op_Add, Op_Sub, Op_Mul, Op_Div, Op_Mod, Op_Assign };
enum ModifierKind { Modifier_Arr, Modifier_Slice };
enum ExprKind { Expr_Atom, Expr_Operator, Expr_FuncCall, Expr_ArrIndex };
enum AtomKind { Atom_Variable, Atom_Constant };
enum StatementKind {
  Statement_Empty,
  Statement_Declaration,
  Statement_Definition,
  Statement_Expression,
  Statement_If,
  Statement_Return,
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
  DefaultTypeCount
};

// --- Core Structs ---
struct Modifier {
  ModifierKind kind;
  uint64_t size;
};

struct Type {
  uint64_t type_id;
  std::vector<Modifier> mods;
};

struct Atom {
  AtomKind kind;
  std::string value;
  Type type;
};

struct Expr;

struct FuncCall {
  std::string name;
  std::vector<Expr *> args;
};

struct ArrIndex {
  std::string name;
  std::vector<Expr *> index;
};

struct Expr {
  ExprKind kind;
  union {
    Atom at;
    struct {
      Operator op;
      Expr *left;
      Expr *middle;
      Expr *right;
    };
    FuncCall func_call;
    ArrIndex arr_index;
  };

  // Constructor declarations (Implemented in parser.cpp)
  Expr(ArrIndex ar);
  Expr(FuncCall fn);
  Expr(Atom at);
  Expr(Expr *left, Expr *right, Operator op);
};

// Statement variants
struct DeclarationStatement {
  std::string name;
  Type type;
};
struct DefinitionStatement {
  std::string name;
  Type type;
  Expr *right;
};
struct ExpressionStatement {
  Expr *root;
};
struct ReturnStatement {
  Expr *root;
};
struct Statement;
struct IfStatement {
  Expr *if_condition;
  std::vector<Statement *> if_block_statements;
  std::vector<Expr *> else_if_conditions;
  std::vector<std::vector<Statement *>> else_if_condition_statements;
  std::vector<Statement *> else_statements;
};

struct Statement {
  StatementKind kind;
  union {
    DeclarationStatement *declaration_statement;
    DefinitionStatement *definition_statement;
    ExpressionStatement *expression_statement;
    IfStatement *if_else_statement;
    ReturnStatement *return_statement;
  };

  Statement();
  Statement(DeclarationStatement *s);
  Statement(DefinitionStatement *s);
  Statement(ExpressionStatement *s);
  Statement(IfStatement *s);
  Statement(ReturnStatement *s);
};

struct Parameter {
  std::string name;
  Type type;
};

struct FunctionDefinition {
  std::string name;
  std::optional<Type> return_type;
  std::vector<Parameter> parameters;
  std::vector<Statement *> statements;
};

// --- Global Parser API ---
bool parse_function_definition(Lexer &l, FunctionDefinition &f);
void pretty_print_expr(Expr *root, const std::string &prefix = "",
                       const std::string &prefix_to_pass = "");

#endif
