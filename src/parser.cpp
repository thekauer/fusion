#include "parser.h"
#include "llvm/Support/raw_os_ostream.h"

Token Parser::pop() {
  if (it == end)
    return Token(Token::Eof, (it - 1)->sl);
  return *it++;
}
Token Parser::peek(int n) { return *(it + n); }

Token Parser::expect(Token::Type ty, const std::string &tk) {
  auto t = pop();
  if (t.type != ty)
    Error::ExpectedToken(file, t.sl, "Expected " + tk);
  return t;
}

int precedence(Token::Type op) {
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
std::unique_ptr<Body> Parser::parse()
{
    auto loc = peek().sl;
    std::vector<std::unique_ptr<AstExpr>> body;
    auto top_level_expr = parse_top_level();
    while (top_level_expr) {
        body.push_back(std::move(top_level_expr));
        top_level_expr = parse_top_level();
    }
    return std::make_unique<Body>(loc, std::move(body));
}
std::unique_ptr<AstExpr> Parser::parse_top_level(){   
    //in the future this should be extended to parse classes,mods etc. as well
    return parse_fndecl();
}

std::unique_ptr<FnProto> Parser::parse_fnproto() {

  if (peek().type != Token::Kw && peek().getKw() != Kw_e::Fn)

    return nullptr;
  pop();
  auto namet = pop();
  if (namet.type != Token::Id) {
    serror(Error_e::Unk, "NAMET-FNPROTO");
  }
  auto name = namet.getName();
  // generics
  expect(Token::Lp, "a (");
  // args
  auto arg = parse_var_decl();
  std::vector<std::unique_ptr<VarDeclExpr>> args;
  while (arg) {
    args.push_back(std::move(arg));

    if (peek().type == Token::Comma) {
      pop();
    }

    else {
      if (peek().type != Token::Rp) {
        serror(Error_e::Unk, "expected , or )");
      }
    }
    arg = parse_var_decl();
  }

  expect(Token::Rp, "a )");
  // MAybe return type
  return std::make_unique<FnProto>(namet.sl, name, std::move(args));
}

std::unique_ptr<FnDecl> Parser::parse_fndecl() {
  FnModifiers::Type mods = 0;
  if (peek().type == Token::Kw && peek().getKw() == Extern) {
    pop();
    mods |= FnModifiers::Extern;
  }

  auto proto = parse_fnproto();
  if (!proto)
    return nullptr;
  if (mods & FnModifiers::Extern) {
    if (peek().type != Token::N) {
      serror(Error_e::Unk, "should be new line");
    }
    return std::make_unique<FnDecl>(peek().sl, std::move(proto));
  }
  auto body = parse_body();
  if (!body) {
      return nullptr;
  }
  
  auto ret = std::make_unique<FnDecl>(proto->sl, std::move(proto), std::move(body), mods);
  return ret;
}

std::unique_ptr<ValExpr> Parser::parse_valexpr() {
  auto t = peek();
  if (t.type == Token::Lit) {
    pop();
    return std::make_unique<ValExpr>(t.sl, t.getValue());
  }
  if (t.type == Token::Kw) {
    if (t.getKw() == Kw_e::True) {
      pop();
      return std::make_unique<ValExpr>(t.sl, Lit(true));
    }
    if (t.getKw() == Kw_e::False) {
      pop();
      return std::make_unique<ValExpr>(t.sl, Lit(false));
    }
  }
  return nullptr;
}

std::unique_ptr<AstExpr> Parser::parse_primary() {
  if (peek().type == Token::N)
    pop();

  std::unique_ptr<AstExpr> expr = parse_valexpr();
  if (expr)
    return expr;
  expr = parse_fncall();
  if (expr)
    return expr;
  expr = parse_ifstmt();
  if (expr)
      return expr;
  expr = parse_return();
  if (expr)
      return expr;
  expr = parse_var_decl();
  if (expr)
    return expr;
  return parse_var();
}
std::unique_ptr<AstExpr> Parser::parse_binary(std::unique_ptr<AstExpr> lhs,
                                              int p) {
  /*
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
    return std::make_unique<BinExpr>(loc, op, move(lhs), move(rhs));
  }
  auto np = pre(peek(1).type); // peek
  if (np == -1) {
    return std::make_unique<BinExpr>(loc, op, move(lhs), move(rhs));
  }
  if (tp >= np) {
    return parse_binary(
        std::make_unique<BinExpr>(loc, op, move(lhs), move(rhs)));
  }

  return std::make_unique<BinExpr>(loc, op, move(lhs), parse_binary(move(rhs)));
  */
  while (it != end) {
    auto op = peek().type;
    auto loc = peek().sl;
    auto TokPrec = precedence(op);
    if (TokPrec < p) {
      return lhs;
    }

    pop(); // pop the op
    auto rhs = parse_primary();
    if (!rhs) {
      return nullptr;
    }
    if (it == end) {
      return std::make_unique<BinExpr>(loc, op, std::move(lhs), std::move(rhs));
    }
    auto next_token = peek().type;
    auto NextPrec = precedence(next_token);
    if (TokPrec < NextPrec) {
      rhs = parse_binary(std::move(rhs), TokPrec + 1);
      if (!rhs) {
        return nullptr;
      }
    }

    lhs = std::make_unique<BinExpr>(loc, op, std::move(lhs), std::move(rhs));
  }
  return lhs;
}

std::unique_ptr<TypeExpr> Parser::parse_type_expr() {
  // implement references and options

  // eh
  if (peek().type != Token::Kw) {
    return nullptr;
  }
  auto sl = peek().sl;
  switch (pop().getKw()) {
  case Kw_e::I32:
    return std::make_unique<TypeExpr>(sl, QualType(Type::get_i32()));
  case Kw_e::I8:
    return std::make_unique<TypeExpr>(sl, QualType(Type::get_i8()));
  case Kw_e::I16:
    return std::make_unique<TypeExpr>(sl, QualType(Type::get_i16()));
  case Kw_e::I64:
    return std::make_unique<TypeExpr>(sl, QualType(Type::get_i64()));
  case Kw_e::Bool:
    return std::make_unique<TypeExpr>(sl, QualType(Type::get_bool()));
  case Kw_e::Drop:
    return std::make_unique<TypeExpr>(sl);
  default:
    return nullptr;
  }
  return nullptr;
}



std::unique_ptr<VarDeclExpr> Parser::parse_var_decl() {
  if (peek().type != Token::Id || peek(1).type != Token::DoubleDot) {
    return nullptr;
  }
  auto id = pop();
  pop(); // DoubleDot
  std::unique_ptr<TypeExpr> ty = parse_type_expr();
  if (!ty) {
    serror(Error_e::Unk, "Expected type expr");
  }
  return std::make_unique<VarDeclExpr>(peek().sl, id.getName(), ty->ty);
}

std::unique_ptr<AstExpr> Parser::parse_expr() {
  auto lhs = parse_primary();
  if (lhs && lhs->type != AstType::IfStmt && lhs->type!=AstType::ReturnStmt) // fix this
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
  auto name = namet.getName();

  // args
  std::vector<std::unique_ptr<AstExpr>> args;
  auto arg = parse_expr();
  args.push_back(std::move(arg));
  while (peek().type == Token::Comma) {
    pop();
    auto a = parse_expr();
    args.push_back(std::move(a));
  }
  expect(Token::Rp, "a )");
  return std::make_unique<FnCall>(namet.sl, name, std::move(args));
}

std::unique_ptr<VarExpr> Parser::parse_var() {
  auto name = peek();
  if (name.type != Token::Id) {
    return nullptr;
  }
  pop();

  return std::make_unique<VarExpr>(name.sl, name.getName());
}

std::unique_ptr<IfStmt> Parser::parse_ifstmt() {
  if (peek().type == Token::Kw && peek().getKw() == Kw_e::If) {
    auto loc = pop().sl; // pop if
    auto cond = parse_expr();
    if (!cond) {
        Error::ImplementMe("Expected expression after if statement.");
    }
    auto body = parse_body();
    if (!body) {
        return nullptr;
    }
    //else
    if (peek().type == Token::Kw && peek().getKw() == Kw_e::Else) {
        pop(); // pop the else
        auto else_body = parse_body();
        return std::make_unique<IfStmt>(loc,std::move(cond), std::move(body), std::move(else_body));
    }

    return std::make_unique<IfStmt>(loc, std::move(cond), std::move(body));
  }
  return nullptr;
}

std::unique_ptr<ImportExpr> Parser::parse_import() {
  if (peek().type != Token::Kw)
    return nullptr;
  if (peek().getKw() != Kw_e::Import)
    return nullptr;
  pop();
  if (peek().type == Token::Id) {
    auto loc = peek().sl;
    return std::make_unique<ImportExpr>(loc, pop().getName());
  } // else kéne egy id error
  if (pop().type != Token::N) {
    serror(Error_e::Unk, "expected a new line");
  }
  return nullptr;
}

std::unique_ptr<Body> Parser::parse_body()
{
    auto loc = peek().sl;
    if (pop().type != Token::Gi) {
        Error::EmptyFnBody(file, peek().sl);
    }
    std::vector<std::unique_ptr<AstExpr>> body;

    while (peek().type != Token::Li) {
        auto expr = parse_expr();
        if (expr) {
            body.push_back(std::move(expr));
        }
        else {
            Error::ImplementMe("expr in body is null");
            return nullptr;;    
        }
    }
    pop(); //pop the Li
    return std::make_unique<Body>(loc, std::move(body));
}

std::unique_ptr<ReturnStmt> Parser::parse_return() {
    if (peek().type == Token::Kw && peek().getKw() == Kw_e::Return) {
        auto sl = pop().sl; // pop the return
        auto expr = parse_expr();
        return std::make_unique<ReturnStmt>(sl, std::move(expr));
    }
    return nullptr;
}

void Body::pretty_print() const {
    for (const auto& line : body) {
        for(int i=0;i<sl.indent;i++)
        llvm::outs() << " ";
        line->pretty_print();
        llvm::outs() << "\n";
    }
}

void FnProto::pretty_print() const {
  llvm::outs() << "fn " << name << "(";
  for (const auto &arg : args) {
    arg->pretty_print();
    llvm::outs() << ",";
  }
  if (args.size() == 0)
    llvm::outs() << "(";
  llvm::outs() << "\b)";
}

void FnDecl::pretty_print() const {
  proto->pretty_print();
  llvm::outs() << "\n";
  body->pretty_print();
  llvm::outs() << "\n";
}

void ValExpr::pretty_print() const { 
    switch (val.ty.get_type_ptr()->get_typekind()) {
    case Type::Integral: {
        switch ( static_cast<const IntegralType*>(val.ty.get_type_ptr())->ty ) {
        case IntegralType::I8:
            llvm::outs() << val.as.i8;
            return;
        case IntegralType::I16:
            llvm::outs() << val.as.i16;
            return;
        case IntegralType::I32:
            llvm::outs() << val.as.i32;
            return;
        case IntegralType::I64:
            llvm::outs() << val.as.i64;
            return;
        case IntegralType::ISize:
            llvm::outs() << val.as.i64;
            return;
        case IntegralType::U8:
            llvm::outs() << val.as.u8;
            return;
        case IntegralType::U16:
            llvm::outs() << val.as.u16;
            return;
        case IntegralType::U32:
            llvm::outs() << val.as.u32;
            return;
        case IntegralType::U64:
            llvm::outs() << val.as.u64;
            return;
        case IntegralType::USize:
            llvm::outs() << val.as.u64;
            return;
        }
        case Bool:
            if (val.as.b) {
                llvm::outs() << "true";
            }
            else {
                llvm::outs() << "false";
            }
            return;
    }
    default:
        llvm::outs() << "val";
    }
}

void VarDeclExpr::pretty_print() const {
  llvm::outs() << name << " : " << ty.get_type_ptr()->get_name().data();
}
void VarExpr::pretty_print() const { llvm::outs() << name; }
void TypeExpr::pretty_print() const {
  llvm::outs() << ty.get_type_ptr()->get_name().data();
}
void FnCall::pretty_print() const {
  llvm::outs() << name << "(";
  for (const auto &arg : args) {
      if (arg) {
          arg->pretty_print();
          llvm::outs() << ",";
      }
  }
  if (args.size() == 0)
    llvm::outs() << "(";
  llvm::outs() << "\b)\n";
}

void BinExpr::pretty_print() const {
  lhs->pretty_print();
  llvm::outs() << " ";
  switch (op) {
  case Token::Add:
    llvm::outs() << "+";
    break;
  case Token::Eq:
    llvm::outs() << "=";
    break;
  case Token::Mul:
    llvm::outs() << "*";
    break;
  default:
    llvm::outs() << " op ";
    break;
  }
  llvm::outs() << " ";
  rhs->pretty_print();
}

void RangeExpr::pretty_print() const {
  if (begin)
    begin->pretty_print();
  llvm::outs() << "..";
  if (end)
    end->pretty_print();
}

void IfStmt::pretty_print() const {
  llvm::outs() << "if ";
  condition->pretty_print();
  llvm::outs() << "\n";
  body->pretty_print();
  if (else_body) {
      llvm::outs() << "else\n";
      else_body->pretty_print();
  }
}

void ImportExpr::pretty_print() const { llvm::outs() << "import " << module; }

void ReturnStmt::pretty_print() const {
    llvm::outs() << "return ";
    expr->pretty_print();
}