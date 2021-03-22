#include "parser.h"
#include "llvm/Support/raw_os_ostream.h"

Token Parser::pop() {
  if (it == end)
    return Token(Token::Eof, (it - 1)->sl);
  return *it++;
}
Token Parser::peek(int n) { return it!=end?*(it + n):Token(Token::Null,(it-1)->sl); }

void Parser::consume_li() {
  if (it == end)
    return;
  if (peek().sl.li == 0) {
    pop(); // pop the Li
  } else {
    it->sl.li--;
  }
}

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
Type *lookup_type(AstExpr *head, const std::string& name) { 
    if (!head) {
    return nullptr;
    }
    switch (head->type) {
    case AstType::ClassStmt: {
      auto* pclass = head->cast<ClassStmt>();
      if (pclass->name->name == name) {
        return const_cast<Type*>(pclass->ty.get());
      } else {
        return lookup_type(pclass, name);
      }
    }
    case AstType::Body: {
      Type *res=nullptr;
      auto *pbody = head->cast<Body>();
      for (auto const &line : pbody->body) {
        res = lookup_type(line.get(),name);
        if (res) {
          return res;
        }
      }
      return nullptr;
    }
    
  default:
      return nullptr;
  }
    
    return nullptr;
}
std::unique_ptr<Type>
ClassStmt::get_class_type(const std::unique_ptr<VarExpr> &name,
                          const std::unique_ptr<Body> &body) {
  std::vector<QualType> types;
  for (auto const &line : body->body) {
    if (line->type == AstType::VarDeclExpr) {
      auto t = line->cast<VarDeclExpr>()->ty;
      types.push_back(t);
    }
  }
  return std::make_unique<StructType>(name->name, std::move(types));
}

std::unique_ptr<Body> Parser::parse() {
  auto loc = peek().sl;
  std::vector<std::unique_ptr<AstExpr>> body;
  auto top_level_expr = parse_top_level();
  while (top_level_expr) {
    body.push_back(std::move(top_level_expr));
    top_level_expr = parse_top_level();
  }
  auto ret = std::make_unique<Body>(loc, std::move(body));
  ctx.head = ret.get();
  return ret;
}
std::unique_ptr<AstExpr> Parser::parse_top_level() {
  if (it == end) {
    return nullptr;
  }
  std::unique_ptr<AstExpr> tle = parse_fndecl();
  if (tle)
    return tle;
  tle = parse_class();
  if (tle)
    return tle;
  return nullptr;
}

std::unique_ptr<FnProto> Parser::parse_fnproto() {

  if (peek().type == Token::Kw && peek().getKw() == Kw_e::Fn) {
    pop();
    auto namet = pop();
    if (namet.type != Token::Id) {
      Error::ExpectedToken(file, namet.sl, "expected function name");
      return nullptr;
    }
    auto name = namet.getName();
    // implement generics
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
  return nullptr;
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

  auto ret = std::make_unique<FnDecl>(proto->sl, std::move(proto),
                                      std::move(body), mods);
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
    if (peek().type == Token::Id) {
      auto name = pop().getName();
      return std::make_unique<TypeExpr>(peek().sl,
                                        QualType(*new ResolveType(name)));
    }
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
  if (lhs && lhs->type != AstType::IfStmt &&
      lhs->type != AstType::ReturnStmt) // fix this
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
    // else
    if (peek().type == Token::Kw && peek().getKw() == Kw_e::Else) {
      pop(); // pop the else
      auto else_body = parse_body();
      if (!else_body) {
        return nullptr;
      }
      return std::make_unique<IfStmt>(loc, std::move(cond), std::move(body),
                                      std::move(else_body));
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

std::unique_ptr<Body> Parser::parse_body() {
  auto loc = peek().sl;
  if (pop().type != Token::Gi) {
    Error::EmptyFnBody(file, peek().sl);
  }
  std::vector<std::unique_ptr<AstExpr>> body;

  while (!is_end_of_body()) {
    auto expr = parse_expr();
    if (expr) {
      body.push_back(std::move(expr));
    } else {
      Error::ImplementMe("expr in body is null");
      return nullptr;
    }
  }
  consume_li();
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
std::unique_ptr<ClassStmt> Parser::parse_class() {
  if (peek().type == Token::Kw && peek().getKw() == Kw_e::Class) {
    auto sl = pop().sl; // pop the class
    auto id = parse_var();
    if (!id) {
      Error::ImplementMe("class must have a name");
    }
    auto body = parse_class_body();
    if (!body) {
      return nullptr;
    }
    auto class_ty = ClassStmt::get_class_type(id, body);
    return std::make_unique<ClassStmt>(sl, std::move(id), std::move(body),std::move(class_ty));
  }
  return nullptr;
}

std::unique_ptr<Body> Parser::parse_class_body() {
  auto loc = peek().sl;
  if (pop().type != Token::Gi) {
    Error::EmptyFnBody(file, peek().sl);
  }
  std::vector<std::unique_ptr<AstExpr>> body;

  while (!is_end_of_body()) {
    if (peek().type == Token::N) {
      pop();
    }
    auto expr = parse_inside_class();
    if (expr) {
      body.push_back(std::move(expr));
    } else {
      Error::ImplementMe("expr in body is null");
      return nullptr;
      ;
    }
  }
  consume_li();
  return std::make_unique<Body>(loc, std::move(body));
}

std::unique_ptr<AstExpr> Parser::parse_inside_class() {
  std::unique_ptr<AstExpr> expr = parse_fndecl();
  if (expr)
    return expr;
  return parse_var_decl();
}
bool Parser::is_end_of_body() {
  return it==end ||  peek().type == Token::Li || peek().type == Token::Null;
}

Body::Body(const SourceLocation &sl,
           std::vector<std::unique_ptr<AstExpr>> body)
    : AstExpr(AstType::Body, sl), body(std::move(body)) {}

Stmt::Stmt(const SourceLocation &sl, AstType type,
           std::unique_ptr<Body> body) : AstExpr(type,sl), body(std::move(body)){}
Expr::Expr(const SourceLocation &sl, AstType type, QualType ty) : AstExpr(type,sl),ty(ty) {}
VarDeclExpr::VarDeclExpr(const SourceLocation &sl, const std::string &name) : AstExpr(AstType::VarDeclExpr,sl),name(name) {}
VarDeclExpr::VarDeclExpr(const SourceLocation &sl, const std::string &name,
                         QualType &ty) : AstExpr(AstType::VarDeclExpr,sl),name(name),ty(ty) {}

FnProto::FnProto(const SourceLocation &sl, const std::string &name,
                 std::unique_ptr<AstExpr> ret) : AstExpr(AstType::FnProto,sl),ret(std::move(ret)),name(name) {}

FnProto::FnProto(const SourceLocation &sl, const std::string &name,
                 std::vector<std::unique_ptr<VarDeclExpr>> args,
                 std::unique_ptr<AstExpr> ret)
    : AstExpr(AstType::FnProto,sl), ret(std::move(ret)), args(std::move(args)),
      name(name) {}
FnDecl::FnDecl(const SourceLocation &sl, std::unique_ptr<FnProto> proto,
               std::unique_ptr<Body> body, FnModifiers::Type mods)
    : AstExpr(AstType::FnDecl, sl), mods(mods),
      proto(std::move(proto)) ,body(std::move(body)) {}
FnDecl::FnDecl(const SourceLocation &sl, std::unique_ptr<FnProto> proto)
    : AstExpr(AstType::FnDecl, sl), mods(FnModifiers::Extern),
      proto(std::move(proto)) {}

ValExpr::ValExpr(const SourceLocation &sl, Lit val) : AstExpr(AstType::ValExpr,sl),val(val){}
VarExpr::VarExpr(const SourceLocation &sl, const std::string &name) : AstExpr(AstType::VarExpr,sl),name(name) {}
TypeExpr::TypeExpr(const SourceLocation &sl, QualType ty) : AstExpr(AstType::TypeExpr,sl),ty(ty) {}
TypeExpr::TypeExpr(const SourceLocation &sl) : AstExpr(AstType::TypeExpr,sl) {}
FnCall::FnCall(const SourceLocation &sl, const std::string &name) : AstExpr(AstType::FnCall,sl),name(name) {}
FnCall::FnCall(const SourceLocation &sl, const std::string &name,
       std::vector<std::unique_ptr<AstExpr>> args)
    : AstExpr(AstType::FnCall,sl), name(name), args(std::move(args)) {}
BinExpr::BinExpr(const SourceLocation &sl, Token::Type op,
                 std::unique_ptr<AstExpr> lhs, std::unique_ptr<AstExpr> rhs) : AstExpr(AstType::BinExpr,sl),lhs(std::move(lhs)),rhs(std::move(rhs)),op(op) {}
RangeExpr::RangeExpr(const SourceLocation &sl, std::unique_ptr<ValExpr> begin,
                     std::unique_ptr<ValExpr> end) : AstExpr(AstType::RangeExpr,sl),begin(std::move(begin)),end(std::move(end)){}
IfStmt::IfStmt(const SourceLocation &sl, std::unique_ptr<AstExpr> condition,
               std::unique_ptr<Body> body) : AstExpr(AstType::IfStmt,sl),condition(std::move(condition)),body(std::move(body)){}
IfStmt::IfStmt(const SourceLocation &sl, std::unique_ptr<AstExpr> condition,
               std::unique_ptr<Body> body, std::unique_ptr<Body> else_body)
    : AstExpr(AstType::IfStmt, sl), condition(std::move(condition)),
      body(std::move(body)), else_body(std::move(else_body)) {}
ReturnStmt::ReturnStmt(const SourceLocation &sl,
                       std::unique_ptr<AstExpr> expr)
    : AstExpr(AstType::ReturnStmt, sl), expr(std::move(expr)) {}
ImportExpr::ImportExpr(const SourceLocation &sl, const std::string &module) : AstExpr(AstType::ImportExpr,sl),module(module) {}
ClassStmt::ClassStmt(const SourceLocation &sl, std::unique_ptr<VarExpr> name,
                     std::unique_ptr<Body> body, std::unique_ptr<Type> ty)
    : AstExpr(AstType::ClassStmt, sl), ty(std::move(ty)),
      body(std::move(body)),
      name(std::move(name)) {}

void Body::pretty_print() const {
  for (const auto &line : body) {
    for (int i = 0; i < sl.indent; i++)
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
    switch (static_cast<const IntegralType *>(val.ty.get_type_ptr())->ty) {
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
  case IntegralType::Bool:
    if (val.as.b) {
      llvm::outs() << "true";
    } else {
      llvm::outs() << "false";
    }
    return;
  }
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

void ClassStmt::pretty_print() const {
  llvm::outs() << "class " << name->name << "\n";
  body->pretty_print();
}