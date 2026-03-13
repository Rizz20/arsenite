#include "parser.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#define sig_global "$"
#define sig_local "%"
#define sig_label "@"

std::string qbe_type(Type type) {
  uint64_t type_id = type.type_id;
  for (auto mod : type.mods) {
    switch (mod.kind) {
    default:
      return "l";
      break;
    }
  }
  switch (type_id) {
  case DefaultType_u64:
    return "l";
  case DefaultType_u32:
    return "w";
  case DefaultType_u8:
    return "w";
  case DefaultType_u16:
    return "w";
  case DefaultType_i8:
    return "w";
  case DefaultType_i16:
    return "w";
  case DefaultType_i32:
    return "w";
  case DefaultType_i64:
    return "l";
  case DefaultType_f32:
    return "s";
  case DefaultType_f64:
    return "d";
  case DefaultType_char8:
    return "w";
  case DefaultType_char16:
    return "w";
  case DefaultType_char32:
    return "w";
  case DefaultType_string:
    return "l";
  case DefaultTypeCount:
    return "w";
  default:
    return "w";
  }
}


static int temp_counter = 0;
std::string next_temp() { return sig_local + std::to_string(temp_counter++); }

void emit_qbe(const FunctionDefinition &f) {
  if (f.name == "main") {
    printf("export ");
  }
  printf("function ");
  Type ret_type = f.return_type.value_or(Type{.type_id = DefaultType_u64});

  std::cout << qbe_type(ret_type) << " ";

  std::cout << sig_global << f.name;

  // parameter ir
  std::cout << "(";
  long unsigned int i = 0;
  for (auto param : f.parameters) {
    auto param_type = param.type;
    std::cout << qbe_type(param_type) << " ";
    std::cout << sig_local << param.name;
    if (i != f.parameters.size() - 1) {
      std::cout << ", ";
    }
    i++;
  }
  std::cout << ") ";

  std::cout << "{\n";
  std::cout << "@start\n";

  // statement IR
  for (auto statement : f.statements) {
  }

  std::cout << "}\n";
}

int main() {
  // Read the source file
  std::ifstream file("main.at");
  if (!file.is_open())
    return 1;

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
