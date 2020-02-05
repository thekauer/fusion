#include "parser.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include <csignal>

Token Parser::pop() {
  if (it == end)
    return Token(Token::Eof, (--it)->sl);
  return *it++;
}
Token Parser::peek(int n) { return *(it + n); }

Token Parser::expect(Token::Type ty, const std::string &tk) {
  auto t = pop();
  if (t.type != ty)
    serror(Error_e::ExpectedToken, "Expected a(n) " + tk /*, t.sl*/);
  return t;
}

int pre(Token::Type op) {
  switch (op) {
  case Token::Eq:
    return 1;
  case Token::Sub:
  case Token::Add:
    return 10;
  case Token::Mul:
  case Token::Div:
    return 20;

  default:
    return -1;
  }
  return -1;
}

std::unique_ptr<FnProto> Parser::parse_fnproto() {

  auto t = peek();
  if (t.type != Token::Kw && t.getKw()!=Kw_e::Fn)
    return nullptr;
  pop();
  auto name = expect(Token::Id, "identifier");
  // generics
  expect(Token::Lp, "(");
  // args
  auto arg = parse_arg();
  std::vector<std::unique_ptr<VarDeclExpr>> args;
  while(arg) {
    args.push_back(std::move(arg));
    if(peek().type==Token::Comma)pop();
    else {
      if(peek().type!=Token::Rp) {
        serror(Error_e::Unk,"expected , or )");
      }
    }
    arg = parse_arg();
  }

  expect(Token::Rp, ")");
  // MAybe return type
  return std::make_unique<FnProto>(name, std::move(args));
}

std::unique_ptr<FnDecl> Parser::parse_fndecl() {
  auto fn_indent = peek().sl.indent;
  auto proto = parse_fnproto();
  if (!proto)
    return nullptr;
  expect(Token::Gi, "greater indentation");
  ++indent;

  std::vector<std::unique_ptr<AstExpr>> body;
  auto expr = parse_expr();
  if (!expr)
    serror(Error_e::EmptyFnBody, "Empty function body" /*, peek().sl*/);
  while (expr) {
    body.push_back(std::move(expr));

    if (peek().sl.indent <= fn_indent) {
      break;
    }
    if (peek().type == Token::Gi) {
      pop();
      ++indent;
    }
    if (peek().type == Token::Li) {
      pop();
      --indent;
    }
    if (peek().type == Token::N) {
      pop();
    }

    expr = parse_expr();
  }
  auto ret = std::make_unique<FnDecl>(move(proto), move(body));
  return ret;
}

std::unique_ptr<ValExpr> Parser::parse_valexpr() {
  auto t = peek();
  if (t.type == Token::Lit) {
    pop();
    return std::make_unique<ValExpr>(t.getValue());
  }
  return nullptr;
}

std::unique_ptr<AstExpr> Parser::parse_primary() {
  if (peek().type == Token::N)
    pop();
  
  std::unique_ptr<AstExpr> expr = parse_range_expr();
  if (expr)
    return expr;
  expr = parse_valexpr();
  if(expr)
    return expr;
  expr = parse_fncall();
  if (expr)
    return expr;
  expr = parse_var_decl();
  if (expr)
    return expr;
  return parse_var();
  
  }
std::unique_ptr<AstExpr> Parser::parse_binary(std::unique_ptr<AstExpr> lhs,
                                              int p) {
  if (!lhs)
    return nullptr;
  if (it == end)
    return lhs;
  auto op = peek().type;
  // check if op is actually an operator
  if (pre(op) == -1)
    return lhs;
  pop();
  auto tp = pre(op);
  auto rhs = parse_primary();
  if (it == end) {
    return std::make_unique<BinExpr>(op, move(lhs), move(rhs));
  }
  auto np = pre(peek(1).type); // peek
  if (np == -1) {
    return std::make_unique<BinExpr>(op, move(lhs), move(rhs));
  }
  if (tp >= np) {
    return parse_binary(std::make_unique<BinExpr>(op, move(lhs), move(rhs)));
  }

  return std::make_unique<BinExpr>(op, move(lhs), parse_binary(move(rhs)));
}

std::unique_ptr<TypeExpr> Parser::parse_type_expr() {
  if (peek().type != Token::Kw) {
    return nullptr;
  }
  switch (pop().getKw()) {
  case Kw_e::I32:
    return std::make_unique<TypeExpr>(Type::getI32());
  case Kw_e::I8:
    return std::make_unique<TypeExpr>(Type::getI8());
  case Kw_e::I16:
    return std::make_unique<TypeExpr>(Type::getI16());
  case Kw_e::I64:
    return std::make_unique<TypeExpr>(Type::getI64());
  case Kw_e::Drop:
    return std::make_unique<TypeExpr>();
  default:
    return nullptr;
  }
  return nullptr;
}

std::unique_ptr<AstExpr>
Parser::parse_infered_var_decl(const std::string &name) {
  if (peek().type != Token::Eq) {
    serror(Error_e::Unk, "You must assign a value to an infered type decl.");
  } else {
    pop(); // pop the =
    auto val = parse_valexpr();
    if (!val) {
      serror(Error_e::Unk, "expected a literal");
    }
    auto lhs = std::make_unique<VarDeclExpr>(name, val->val.ty);
    return std::make_unique<BinExpr>(Token::Eq, std::move(lhs), std::move(val));
  }
  return nullptr;
}
std::unique_ptr<VarDeclExpr> Parser::parse_arg() {
  auto id = peek();
  if(peek().type!=Token::Id) return nullptr;
  pop();
  auto t = peek();
  if(t.type==Token::Comma || t.type==Token::Rp) {
    llvm::outs() << "Vardecl made here";
    return std::make_unique<VarDeclExpr>(id.getName());
  }
  if(t.type==Token::DoubleDot) {
    pop();
    auto ty = parse_type_expr();
    if(!ty) {
      serror(Error_e::Unk,"Unknown type");
    }
    return std::make_unique<VarDeclExpr>(id.getName(),ty->ty);
  }
  serror(Error_e::Unk,"Parse arg unreachable");
}

std::unique_ptr<AstExpr> Parser::parse_var_decl() {
  if (peek().type != Token::Id) {
    return nullptr;
  }
  auto id = pop();
  if (peek().type == Token::DoubleDot) {
    pop(); // pop Double dot
    if ((peek().type == Token::Underscore) ||
        (peek().type == Token::Kw && peek().getKw() == Kw_e::Drop)) {

      pop(); // pop the _
      return parse_infered_var_decl(id.getName());
    }
    std::unique_ptr<TypeExpr> ty = parse_type_expr();
    // remove this
    if (!ty) {
      // error expected type expr
      return nullptr; // return Infer type
    }
    return std::make_unique<VarDeclExpr>(id.getName(), ty->ty);
  }
  if (peek().type == Token::Eq) {
    return parse_infered_var_decl(id.getName());
  }

  return nullptr;
}

std::unique_ptr<AstExpr> Parser::parse_expr() {
  auto lhs = parse_primary();
  if (lhs)
    return parse_binary(std::move(lhs));
  return lhs;
}

std::unique_ptr<FnCall> Parser::parse_fncall() {
  auto name = peek();
  if (name.type != Token::Id)
    return nullptr;
  if (peek(1).type != Token::Lp) {
    return nullptr;
  }
  pop(); // pop name
  pop(); // pop (

  // args
  std::vector<std::unique_ptr<AstExpr>> args;
  auto arg = parse_expr();
  args.push_back(std::move(arg));
  expect(Token::Rp, ")");
  return std::make_unique<FnCall>(name.getName(), std::move(args));
}

std::unique_ptr<VarExpr> Parser::parse_var() {
  auto name = peek();
  if (name.type != Token::Id) {
    return nullptr;
  }
  pop();

  return std::make_unique<VarExpr>(name.getName());
}

std::unique_ptr<ValExpr> Parser::pop_integer() {
  if(peek().type==Token::Lit) {
    if(peek().getValue().ty->isIntegerType()) {
      return std::make_unique<ValExpr>(pop().getValue());
    }//else only integer types are allowed
  }
  return nullptr;
}

std::unique_ptr<RangeExpr> Parser::parse_range_expr() {
  std::unique_ptr<ValExpr> begin,end;
   if(peek(1).type==Token::DotDot) {
    begin = pop_integer();
  }
  if(peek().type==Token::DotDot) {
    pop();
  } else 
    return nullptr;
  end=pop_integer();
  return std::make_unique<RangeExpr>(std::move(begin),std::move(end));
}

std::unique_ptr<IfExpr> Parser::parse_if_expr() {
  if(peek().type==Token::Kw && peek().getKw()==Kw_e::If) {
    pop(); // pop if
    auto ret = std::make_unique<IfExpr>(std::move(parse_expr()));
    //parse fn body
  }
  return nullptr;
}




void FnProto::pretty_print() {
  llvm::outs() << "fn " << name.getName() << "(";
  for (const auto &arg : args) {
    arg->pretty_print();
    llvm::outs() << ",";
  }
  if (args.size() == 0)
    llvm::outs() << "(";
  llvm::outs() << "\b)";
}

void FnDecl::pretty_print() {
  proto->pretty_print();
  llvm::outs() << "\n";
  for (const auto &b : body) {
    llvm::outs() << " ";
    b->pretty_print();
  }
}

void ValExpr::pretty_print() { llvm::outs() << "val"; }

void VarDeclExpr::pretty_print() {
  llvm::outs() << name << " : " << ty->getName();
  llvm::outs() << "\n";
}
void VarExpr::pretty_print() { llvm::outs() << name; }
void TypeExpr::pretty_print() {llvm::outs()<< ty->getName(); }
void FnCall::pretty_print() {
  llvm::outs() << name << "(";
  for (const auto &arg : args) {
    arg->pretty_print();
    llvm::outs() << ",";
  }
  if (args.size() == 0)
    llvm::outs() << "(";
  llvm::outs() << "\b)\n";
}

void BinExpr::pretty_print() {
  lhs->pretty_print();
  llvm::outs() << " op ";
  rhs->pretty_print();
}

void RangeExpr::pretty_print() {
  if(begin)
  begin->pretty_print();
  llvm::outs() << "..";
  if(end)
  end->pretty_print();
}

void IfExpr::pretty_print() {
  llvm::outs() << "if ";
  condition->pretty_print();
  }

  