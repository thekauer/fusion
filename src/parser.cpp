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

  if (peek().type != Token::Kw && peek().getKw()!=Kw_e::Fn)

    return nullptr;
  pop();
  auto namet = pop();
  if(namet.type!=Token::Id) {
    serror(Error_e::Unk,"NAMET-FNPROTO");
  }
  auto name = namet.getName();
  // generics
  expect(Token::Lp, "(");
  // args
  auto arg = parse_arg();
  std::vector<std::unique_ptr<VarDeclExpr>> args;
  while (arg) {
    args.push_back(std::move(arg));

    if(peek().type==Token::Comma){
      pop();
     }

    else {
      if (peek().type != Token::Rp) {
        serror(Error_e::Unk, "expected , or )");
      }
    }
    arg = parse_arg();
  }

  expect(Token::Rp, ")");
  // MAybe return type
  return std::make_unique<FnProto>(namet.sl,name, std::move(args));
}

std::unique_ptr<FnDecl> Parser::parse_fndecl() {
  FnModifiers::Type mods=0;
  if(peek().type==Token::Kw && peek().getKw()==Extern) {
    pop();
    mods|=FnModifiers::Extern;
  }

  auto fn_indent = peek().sl.indent;
  auto proto = parse_fnproto();
  if (!proto)
    return nullptr;
  if(mods&FnModifiers::Extern) {
    if(peek().type!=Token::N) {
      serror(Error_e::Unk,"should be new line");
    }
    return std::make_unique<FnDecl>(peek().sl,move(proto));
  }
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
  auto ret = std::make_unique<FnDecl>(proto->sl,move(proto), move(body),mods);
  return ret;
}

std::unique_ptr<ValExpr> Parser::parse_valexpr() {
  auto t = peek();
  if (t.type == Token::Lit) {
    pop();
    return std::make_unique<ValExpr>(t.sl,t.getValue());
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
  auto loc = peek().sl;
  // check if op is actually an operator
  if (pre(op) == -1)
    return lhs;
  pop();
  auto tp = pre(op);
  auto rhs = parse_primary();
  if (it == end) {
    return std::make_unique<BinExpr>(loc,op, move(lhs), move(rhs));
  }
  auto np = pre(peek(1).type); // peek
  if (np == -1) {
    return std::make_unique<BinExpr>(loc,op, move(lhs), move(rhs));
  }
  if (tp >= np) {
    return parse_binary(std::make_unique<BinExpr>(loc,op, move(lhs), move(rhs)));
  }

  return std::make_unique<BinExpr>(loc,op, move(lhs), parse_binary(move(rhs)));
}

std::unique_ptr<TypeExpr> Parser::parse_type_expr() {
  Type::By pass = Type::Val;
  if(peek().type==Token::Mul) {
    pass=Type::Ptr;
    pop();
  }
  if(peek().type==Token::And) {
    pass=Type::Ref;
    pop();
  }
  if (peek().type != Token::Kw) {
    return nullptr;
  }
  auto sl = peek().sl;
  switch (pop().getKw()) {
  case Kw_e::I32:
    return std::make_unique<TypeExpr>(sl,Type::getI32()->setBy(pass));
  case Kw_e::I8:
    return std::make_unique<TypeExpr>(sl,Type::getI8()->setBy(pass));
  case Kw_e::I16:
    return std::make_unique<TypeExpr>(sl,Type::getI16()->setBy(pass));
  case Kw_e::I64:
    return std::make_unique<TypeExpr>(sl,Type::getI64()->setBy(pass));
  case Kw_e::Drop:
    return std::make_unique<TypeExpr>(sl);
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
    auto lhs = std::make_unique<VarDeclExpr>(peek().sl,name, val->val.ty);
    return std::make_unique<BinExpr>(peek().sl,Token::Eq, std::move(lhs), std::move(val));
  }
  return nullptr;
}

std::unique_ptr<VarDeclExpr> Parser::parse_arg() {
  auto ty_arg = parse_type_expr();
  if(ty_arg) {
    return std::make_unique<VarDeclExpr>(peek().sl,"",ty_arg->ty);
  }
  
  if(peek().type==Token::Id) {
    std::string id = "arg";//pop().getName();
    pop();//pop name
    if(peek().type==Token::DoubleDot) {
      pop();
      auto ty =  parse_type_expr();
      if(ty) {
        return std::make_unique<VarDeclExpr>(peek().sl,id,ty->ty);
      } else {
        serror(Error_e::Unk,"invalid argument type");
      }
    }
  }
  

  if(peek().type==Token::Rp) {
    return nullptr;
  }
  /*
  auto id = peek();
  if (peek().type != Token::Id)
    return nullptr;
  pop();
  auto t = peek();
  if (t.type == Token::Comma || t.type == Token::Rp) {
    llvm::outs() << "Vardecl made here";
    return std::make_unique<VarDeclExpr>(id.getName());
  }

  if(t.type==Token::DoubleDot) {
    llvm::outs() << "DOUBLE DOT\n";
    pop();
    auto ty = parse_type_expr();
    if (!ty) {
      serror(Error_e::Unk, "Unknown type");
    }

    return std::make_unique<VarDeclExpr>(id.getName(),ty->ty);
  }*/
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
    return std::make_unique<VarDeclExpr>(peek().sl,id.getName(), ty->ty);
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
  auto namet = peek();
  if (namet.type != Token::Id)
    return nullptr;
  if (peek(1).type != Token::Lp) {
    return nullptr;
  }
  pop(); // pop name
  pop(); // pop (
  auto name =namet.getName();

  // args
  std::vector<std::unique_ptr<AstExpr>> args;
  auto arg = parse_expr();
  args.push_back(std::move(arg));
  expect(Token::Rp, ")");
  return std::make_unique<FnCall>(namet.sl,name, std::move(args));
}

std::unique_ptr<VarExpr> Parser::parse_var() {
  auto name = peek();
  if (name.type != Token::Id) {
    return nullptr;
  }
  pop();

  return std::make_unique<VarExpr>(name.sl,name.getName());
}

std::unique_ptr<ValExpr> Parser::pop_integer() {
  if(peek().type==Token::Lit) {
    if(peek().getValue().ty->isIntegerType()) {
      auto loc = peek().sl;
      return std::make_unique<ValExpr>(loc,pop().getValue());
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
  
  auto loc = peek().sl;
  end=pop_integer();
  return std::make_unique<RangeExpr>(loc,std::move(begin),std::move(end));
}

std::unique_ptr<IfExpr> Parser::parse_if_expr() {
  if(peek().type==Token::Kw && peek().getKw()==Kw_e::If) {
    pop(); // pop if
    auto ret = std::make_unique<IfExpr>(peek().sl,parse_expr());
    //parse fn body
  }
  return nullptr;
}

std::unique_ptr<ImportExpr> Parser::parse_import() {
  if(peek().type!=Token::Kw) 
    return nullptr;
  if(peek().getKw()!=Kw_e::Import)
    return nullptr;
  pop();
  if(peek().type==Token::Id) {
    auto loc = peek().sl;
    return std::make_unique<ImportExpr>(loc,pop().getName());
  } //else kéne egy id error
  if(pop().type!=Token::N) {
    serror(Error_e::Unk,"expected a new line");
  }
  return nullptr;
}






void FnProto::pretty_print() {
  llvm::outs() << "fn " << name << "(";
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
}
void VarExpr::pretty_print() { llvm::outs() << name; }
void TypeExpr::pretty_print() { llvm::outs() << ty->getName(); }
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
  switch(op) {
    case Token::Add:
      llvm::outs() << "+";
      break;
    case Token::Eq:
      llvm::outs() << "=";
    default:
     llvm::outs() << " op ";
     break;
  }
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

 void ImportExpr::pretty_print() {
   llvm::outs() << "import "<<module;
 }