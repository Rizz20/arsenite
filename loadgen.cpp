    switch (statement->kind) {
    case Statement_Return: {
      std::cout << "\tret "; // Added indentation for QBE format
      Expr *root = statement->return_statement->root;
      if (root->kind == Expr_Atom) {
        if (root->at.kind == Atom_Constant) {
          std::cout << root->at.value;
        } else if (root->at.kind == Atom_Variable) {
          std::cout << sig_local << root->at.value;
        }
      }
      std::cout << "\n";
      break; // <--- ADD THIS BREAK
    }
    case Statement_Definition: {
      std::cout << "\t"<< sig_local << statement->definition_statement->name << "=";
      //%a=w copy value
      std::cout << qbe_type(statement->definition_statement->type) << " ";

      auto expr=statement->definition_statement->right;
      switch (expr->kind) {
        case Expr_Atom:
          switch (expr->at.kind) {
            case Atom_Constant:
              std::cout << "copy " << expr->at.value;
              break;
            case Atom_Variable:
              std::cout << "add " << sig_local << expr->at.value << ",0";
              break;
          }
          break;
        case Expr_FuncCall:
          std::cout << "call " << sig_global << expr->func_call.name << "(";
          break;
        case Expr_Operator:
          break;
        case Expr_ArrIndex:
          break;
      }

    } break;

    default:
      break;
    }
