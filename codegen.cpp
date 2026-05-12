#include "parser.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <optional>
#include <map>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Host.h>

static std::unique_ptr<llvm::LLVMContext> TheContext;
static std::unique_ptr<llvm::Module> TheModule;
static std::unique_ptr<llvm::IRBuilder<>> Builder;
static std::map<std::string, llvm::AllocaInst *> NamedValues;

llvm::Type *get_llvm_type(::Type type) {
  llvm::Type *base = nullptr;
  uint64_t type_id = type.type_id;
  switch (type_id) {
  case DefaultType_u64: base = llvm::Type::getInt64Ty(*TheContext); break;
  case DefaultType_u32: base = llvm::Type::getInt32Ty(*TheContext); break;
  case DefaultType_u16: base = llvm::Type::getInt16Ty(*TheContext); break;
  case DefaultType_u8:  base = llvm::Type::getInt8Ty(*TheContext); break;
  case DefaultType_i64: base = llvm::Type::getInt64Ty(*TheContext); break;
  case DefaultType_i32: base = llvm::Type::getInt32Ty(*TheContext); break;
  case DefaultType_i16: base = llvm::Type::getInt16Ty(*TheContext); break;
  case DefaultType_i8:  base = llvm::Type::getInt8Ty(*TheContext); break;
  case DefaultType_f32: base = llvm::Type::getFloatTy(*TheContext); break;
  case DefaultType_f64: base = llvm::Type::getDoubleTy(*TheContext); break;
  case DefaultType_char8: base = llvm::Type::getInt8Ty(*TheContext); break;
  case DefaultType_string: base = llvm::PointerType::getUnqual(*TheContext); break; 
  default: base = llvm::Type::getInt32Ty(*TheContext); break;
  }

  for (auto it = type.mods.rbegin(); it != type.mods.rend(); ++it) {
    if (it->kind == Modifier_Arr) {
      base = llvm::ArrayType::get(base, it->size);
    } else if (it->kind == Modifier_Slice) {
      std::vector<llvm::Type*> fields = { llvm::PointerType::getUnqual(*TheContext), llvm::Type::getInt64Ty(*TheContext) };
      base = llvm::StructType::get(*TheContext, fields);
    }
  }
  return base;
}

llvm::Value *codegen_expr(Expr *expr) {
  if (!expr) return nullptr;
  switch (expr->kind) {
  case Expr_Atom: {
    if (expr->at.kind == Atom_Constant) {
        if (expr->at.type.type_id == DefaultType_string) {
            return Builder->CreateGlobalString(expr->at.value, "strtmp");
        }
        // Simple heuristic for type
        llvm::Type *t = get_llvm_type(expr->at.type);
        if (t->isIntegerTy(32) && (std::stoll(expr->at.value) > 0xFFFFFFFFLL)) {
            t = llvm::Type::getInt64Ty(*TheContext);
        }
      return llvm::ConstantInt::get(t, std::stoll(expr->at.value));
    } else if (expr->at.kind == Atom_Variable) {
      llvm::AllocaInst *A = NamedValues[expr->at.value];
      if (!A) {
        std::cerr << "Unknown variable name: " << expr->at.value << std::endl;
        return nullptr;
      }
      return Builder->CreateLoad(A->getAllocatedType(), A, expr->at.value.c_str());
    }
    break;
  }
  case Expr_Operator: {
    llvm::Value *L = codegen_expr(expr->left);
    llvm::Value *R = codegen_expr(expr->right);
    if (!L || !R) return nullptr;

    // Ensure types match for arithmetic
    if (L->getType() != R->getType()) {
        if (L->getType()->isIntegerTy() && R->getType()->isIntegerTy()) {
            if (L->getType()->getIntegerBitWidth() < R->getType()->getIntegerBitWidth()) {
                L = Builder->CreateZExt(L, R->getType());
            } else {
                R = Builder->CreateZExt(R, L->getType());
            }
        }
    }

    switch (expr->op) {
    case Op_Add: return Builder->CreateAdd(L, R, "addtmp");
    case Op_Sub: return Builder->CreateSub(L, R, "subtmp");
    case Op_Mul: return Builder->CreateMul(L, R, "multmp");
    case Op_Div: return Builder->CreateSDiv(L, R, "divtmp");
    case Op_Assign: {
        if (expr->left->kind != Expr_Atom || expr->left->at.kind != Atom_Variable) {
            std::cerr << "Left side of assignment must be a variable" << std::endl;
            return nullptr;
        }
        llvm::AllocaInst *A = NamedValues[expr->left->at.value];
        if (!A) return nullptr;
        
        // Cast R to match A's type if needed
        if (R->getType() != A->getAllocatedType()) {
            if (R->getType()->isIntegerTy() && A->getAllocatedType()->isIntegerTy()) {
                R = Builder->CreateZExtOrTrunc(R, A->getAllocatedType());
            }
        }
        
        Builder->CreateStore(R, A);
        return R;
    }
    default: break;
    }
    break;
  }
  case Expr_FuncCall: {
      llvm::Function *CalleeF = TheModule->getFunction(expr->func_call.name);
      if (!CalleeF) {
          // Try to declare it as a vararg function (like printf) if not found
          // This is a hack to allow calling printf without declaration
          if (expr->func_call.name == "printf") {
              std::vector<llvm::Type *> Args;
              Args.push_back(llvm::PointerType::getUnqual(*TheContext));
              llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(*TheContext), Args, true);
              CalleeF = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "printf", TheModule.get());
          } else {
              std::cerr << "Unknown function referenced: " << expr->func_call.name << std::endl;
              return nullptr;
          }
      }

      std::vector<llvm::Value *> ArgsV;
      for (auto &Arg : expr->func_call.args) {
          ArgsV.push_back(codegen_expr(Arg));
          if (!ArgsV.back()) return nullptr;
      }

      return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
  }
  default:
    break;
  }
  return nullptr;
}

void codegen_statement(Statement *stmt) {
  switch (stmt->kind) {
  case Statement_Declaration: {
    auto decl = stmt->declaration_statement;
    llvm::Function *F = Builder->GetInsertBlock()->getParent();
    llvm::IRBuilder<> TmpB(&F->getEntryBlock(), F->getEntryBlock().begin());
    llvm::AllocaInst *Alloca = TmpB.CreateAlloca(get_llvm_type(decl->type), nullptr, decl->name);
    NamedValues[decl->name] = Alloca;
    break;
  }
  case Statement_Definition: {
    auto def = stmt->definition_statement;
    llvm::Function *F = Builder->GetInsertBlock()->getParent();
    llvm::IRBuilder<> TmpB(&F->getEntryBlock(), F->getEntryBlock().begin());
    llvm::AllocaInst *Alloca = TmpB.CreateAlloca(get_llvm_type(def->type), nullptr, def->name);
    NamedValues[def->name] = Alloca;
    llvm::Value *Val = codegen_expr(def->right);
    if (Val) {
        if (Val->getType() != Alloca->getAllocatedType()) {
            if (Val->getType()->isIntegerTy() && Alloca->getAllocatedType()->isIntegerTy()) {
                Val = Builder->CreateZExtOrTrunc(Val, Alloca->getAllocatedType());
            }
        }
        Builder->CreateStore(Val, Alloca);
    }
    break;
  }
  case Statement_Return: {
    llvm::Value *RetVal = codegen_expr(stmt->return_statement->root);
    llvm::Function *F = Builder->GetInsertBlock()->getParent();
    if (RetVal) {
        if (RetVal->getType() != F->getReturnType()) {
            if (RetVal->getType()->isIntegerTy() && F->getReturnType()->isIntegerTy()) {
                RetVal = Builder->CreateZExtOrTrunc(RetVal, F->getReturnType());
            }
        }
        Builder->CreateRet(RetVal);
    }
    else Builder->CreateRetVoid();
    break;
  }
  case Statement_Expression: {
    codegen_expr(stmt->expression_statement->root);
    break;
  }
  default:
    break;
  }
}

void emit_llvm(const FunctionDefinition &f) {
  NamedValues.clear();
  std::vector<llvm::Type *> arg_types;
  for (auto &param : f.parameters) {
    arg_types.push_back(get_llvm_type(param.type));
  }

  ::Type default_ret = {DefaultType_u64, {}};
  llvm::Type *ret_type_val = get_llvm_type(f.return_type.value_or(default_ret));
  llvm::FunctionType *FT = llvm::FunctionType::get(ret_type_val, arg_types, false);
  llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, f.name, TheModule.get());

  unsigned idx = 0;
  for (auto &arg : F->args()) {
    arg.setName(f.parameters[idx++].name);
  }

  llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", F);
  Builder->SetInsertPoint(BB);

  // Store arguments in stack
  idx = 0;
  for (auto &arg : F->args()) {
      llvm::IRBuilder<> TmpB(&F->getEntryBlock(), F->getEntryBlock().begin());
      llvm::AllocaInst *Alloca = TmpB.CreateAlloca(arg.getType(), nullptr, arg.getName());
      Builder->CreateStore(&arg, Alloca);
      NamedValues[f.parameters[idx++].name] = Alloca;
  }

  for (auto stmt : f.statements) {
    codegen_statement(stmt);
  }

  // Ensure every block has a terminator
  if (!BB->getTerminator()) {
    if (ret_type_val->isVoidTy()) {
      Builder->CreateRetVoid();
    } else if (ret_type_val->isIntegerTy()) {
      Builder->CreateRet(llvm::ConstantInt::get(ret_type_val, 0));
    } else {
      Builder->CreateUnreachable();
    }
  }

  if (llvm::verifyFunction(*F, &llvm::errs())) {
      std::cerr << "Function " << f.name << " failed verification!" << std::endl;
  }
}

void generate_object_file(const std::string &filename) {
  auto TargetTripleString = llvm::sys::getDefaultTargetTriple();
  llvm::Triple TT(TargetTripleString);

  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  std::string Error;
  auto Target = llvm::TargetRegistry::lookupTarget(TT, Error);

  if (!Target) {
    llvm::errs() << Error;
    return;
  }

  auto CPU = "generic";
  auto Features = "";

  llvm::TargetOptions opt;
  auto RM = std::optional<llvm::Reloc::Model>(llvm::Reloc::PIC_);
  auto TheTargetMachine = Target->createTargetMachine(TT, CPU, Features, opt, RM);

  TheModule->setDataLayout(TheTargetMachine->createDataLayout());
  TheModule->setTargetTriple(TT);

  std::error_code EC;
  llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);

  if (EC) {
    llvm::errs() << "Could not open file: " << EC.message();
    return;
  }

  llvm::legacy::PassManager pass;
  auto FileType = llvm::CodeGenFileType::ObjectFile;

  if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
    llvm::errs() << "TheTargetMachine can't emit a file of this type";
    return;
  }

  pass.run(*TheModule);
  dest.flush();
}

int main(int argc, char **argv) {
  std::string input_file = "main.at";
  if (argc > 1) {
    input_file = argv[1];
  }

  TheContext = std::make_unique<llvm::LLVMContext>();
  TheModule = std::make_unique<llvm::Module>("arsenite", *TheContext);
  Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);

  std::ifstream file(input_file);
  if (!file.is_open()) {
    std::cerr << "Could not open file: " << input_file << std::endl;
    return 1;
  }

  std::ostringstream ss;
  ss << file.rdbuf();

  Lexer l = lexer_lex_file(ss.str());
  FunctionDefinition f{};

  if (parse_function_definition(l, f)) {
    emit_llvm(f);
    generate_object_file("output.o");
    TheModule->print(llvm::errs(), nullptr);
    std::cout << "Successfully generated output.o" << std::endl;
    
    // Link to create executable
    std::cout << "Linking output.o to main_exec..." << std::endl;
    int res = system("gcc output.o -o main_exec");
    if (res == 0) {
        std::cout << "Successfully linked main_exec" << std::endl;
    } else {
        std::cerr << "Linking failed with exit code " << res << std::endl;
    }
  } else {
    std::cerr << "Codegen failed: Parsing error." << std::endl;
    return 1;
  }

  return 0;
}
