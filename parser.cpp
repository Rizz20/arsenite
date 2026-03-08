#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <cstdio>
#include <cstdint>
#include <vector>
#include "lexer.h"

#include "lexer.cpp"


// default types



struct Statement {
    int a;
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
    DefaultTypeCount,
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

enum OpAss {
    OpAss_Left,
    OpAss_Right,
};

enum ExprKind {
    Expr_Atom,
    Expr_Operator,
    Expr_FuncCall,
    Expr_ArrIndex,
    Expr_Assign,
};

enum AtomKind {
    Atom_Variable,
    Atom_Constant,
};


struct Type {
    uint64_t type_id;
    std::vector<uint64_t> arr_size;
};

#define TYPE_UNKNOWN (Type {.type_id = DefaultType_Unkown, .arr_size{}})

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

struct ArrIndex {
    std::string name;
    std::vector<Expr*> index;
};

struct AssignExpr{
    std::string name;
    Expr* right;
};

// a[] <- postfix unary
// *a  <- prefix unary
//
// a[][][][] <- multiple
// *******a  <- multiple
// **a[4][2] <- combined

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
        AssignExpr ass_expr; //Expr for assignment operation

    };

    Expr (AssignExpr as)
            :kind(Expr_Assign), ass_expr(as){}

    Expr (ArrIndex ar)
            :kind(Expr_ArrIndex), arr_index(ar){}

    Expr (FuncCall fn)
            : kind(Expr_FuncCall), func_call(fn){}

    Expr (Atom at)
          : kind(Expr_Atom), at(at) {}

    Expr (Expr *left, Expr *right, Operator op)
          : kind(Expr_Operator), op(op), left(left), right(right) {}
};

Expr* parse_primary(Lexer& l);

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

bool is_op(Token t){
    auto it = op_table.find(t.kind);

    return !(it == op_table.end());
}

Expr* parse_expression(Lexer& l, int min_prec){
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

        if(op == Op_Assign){
            if(primary_lhs->kind == Expr_Atom && primary_lhs->at.kind == Atom_Variable){
                AssignExpr as{.name = primary_lhs->at.value, .right = primary_rhs};
                Expr* ep = new Expr(as);
                return ep;
            }
            else{
                std::cout<<"Horrible mistakes have been done"<<std::endl;
                return nullptr;
            }
        }
        else{
            primary_lhs = new Expr(primary_lhs, primary_rhs, op);
        }

    }

    return primary_lhs;

}

void pretty_print_expr(Expr *root, const std::string& prefix, const std::string& prefix_to_pass)
{
    if (root == nullptr) {          // ✓ add this guard
        std::printf("%s<null>\n", prefix.c_str());              //This is fuckass
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
        case Expr_Assign:
            std::printf("Assign: %s =\n", root->ass_expr.name.c_str());
            pretty_print_expr(root->ass_expr.right, prefix_to_pass + "└──╴", prefix_to_pass + "    ");
            break;

    }
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


Expr* parse_function_call(Lexer& l, std::string val){



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



int main() {

    std::ifstream file("main.at");
    std::ostringstream ss;

    ss << file.rdbuf();

    std::string source_code = ss.str();

    Lexer l = lexer_lex_file(source_code);

    Expr *ast = parse_expression(l, 0);
    pretty_print_expr(ast, "", "");

    return 0;
}

