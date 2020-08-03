#pragma once
#include "error.h"
#include "lex.h"
#include "type.h"
#include <memory>

struct FusionCtx; // insted of including "context.h" so we don't include any llvm

enum class AstType : unsigned char {
  FnDecl,
  FnCall,
  FnProto,
  BinExpr,
  ValExpr,
  TypeExpr,
  VarExpr,
  VarDeclExpr,
  RangeExpr,
  IfStmt,
  ImportExpr,
  ModExpr,
  Body,
  ReturnStmt,
  ClassStmt
};
class AstExpr {
public:
  AstType type;
  SourceLocation sl;
  AstExpr(AstType type, const SourceLocation &sl) : type(type), sl(sl){};
  virtual void print_name() const { std::cout << "AstExpr\n"; };
  virtual llvm::Value *codegen(FusionCtx &ctx) const = 0;
  virtual void pretty_print() const = 0;
  virtual ~AstExpr() {}
  int indent = 0;
  template<typename T>
  T* cast() {
      return reinterpret_cast<T*>(this);
  }
};

struct Body : AstExpr {
    std::vector<std::unique_ptr<AstExpr>> body;
    Body(const SourceLocation& sl, std::vector<std::unique_ptr<AstExpr>>&& body) : body(std::move(body)), AstExpr(AstType::Body, sl) {}
    void print_name() const { std::cout << "body\n"; };
    llvm::Value* codegen(FusionCtx& ctx) const override;
    void pretty_print() const override;
};
struct Stmt : AstExpr {
    std::unique_ptr<Body> body;
    Stmt(const SourceLocation& sl,AstType type ,std::unique_ptr<Body>&& body) : body(std::move(body)), AstExpr(type, sl) {};
    virtual void print_name() const;
    virtual llvm::Value* codegen(FusionCtx& ctx) const = 0;
    virtual void pretty_print() const = 0;
    virtual ~Stmt() {};
};
struct Expr : AstExpr {
    QualType ty;
    Expr(const SourceLocation& sl, AstType type, QualType ty) : ty(ty), AstExpr(type, sl) {};
    virtual void print_name() const;
    virtual llvm::Value* codegen(FusionCtx& ctx) const = 0;
    virtual void pretty_print() const = 0;
    virtual ~Expr() {};
};
struct VarDeclExpr : AstExpr {
  std::string name;
  QualType ty;

  VarDeclExpr(const SourceLocation &sl, const std::string &name)
      : AstExpr(AstType::VarDeclExpr, sl), name(name) {}
  VarDeclExpr(const SourceLocation &sl, const std::string &name, QualType &ty)
      : AstExpr(AstType::VarDeclExpr, sl), name(name), ty(ty){};
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void print_name() const override { std::cout << "VarDeclExpr\n"; }
  void pretty_print() const override;
};
struct FnModifiers {
  using Type = unsigned char;
  static const Type Pub = 1;
  static const Type Extern = 2;
  static const Type Export = 4;
  static const Type Inline = 8;
  static const Type Async = 16;
  static const Type Const = 32;
  static const Type Static = 64;
  FnModifiers() = delete;
};
struct FnProto : AstExpr {
  std::unique_ptr<AstExpr> ret;
  std::vector<std::unique_ptr<VarDeclExpr>> args;
  std::string name;
  Linkage linkage = Linkage::Ext;
  Inline is_inline = Inline::Def;
  FnProto(const SourceLocation &sl, const std::string &name,
          std::unique_ptr<AstExpr> ret)

      : AstExpr(AstType::FnProto, sl), ret(std::move(ret)), name(name){};

  FnProto(const SourceLocation &sl, const std::string &name,
          std::vector<std::unique_ptr<VarDeclExpr>> &&args,
          std::unique_ptr<AstExpr> ret = nullptr)
      : AstExpr(AstType::FnProto, sl), ret(std::move(ret)),
        args(std::move(args)), name(name) {}

  void print_name() const override { std::cout << "FnProto\n"; }
  void pretty_print() const override;
  llvm::Value *codegen(FusionCtx &ctx) const override;
};

struct FnDecl : AstExpr {
  FnModifiers::Type mods = 0;
  std::unique_ptr<FnProto> proto;
  std::unique_ptr<Body> body;
  FnDecl(const SourceLocation &sl, std::unique_ptr<FnProto> proto,
         std::unique_ptr<Body> &&body,
         FnModifiers::Type mods = 0)
      : AstExpr(AstType::FnDecl, sl), proto(std::move(proto)), body(std::move(body)),
        mods(mods){};
  FnDecl(const SourceLocation &sl, std::unique_ptr<FnProto> proto)
      : AstExpr(AstType::FnDecl, sl), proto(std::move(proto)),
        mods(FnModifiers::Extern) {}
  void print_name() const override {
    std::cout << "FnDecl: " << proto->name << "\n";
  }
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
};
struct ValExpr : AstExpr {
  Lit val;
  ValExpr(const SourceLocation &sl, Lit val)
      : AstExpr(AstType::ValExpr, sl), val(val) {}
  void print_name() const override { std::cout << "ValExpr\n"; }
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
};
struct VarExpr : AstExpr {
  std::string name;

  VarExpr(const SourceLocation &sl, const std::string &name)
      : AstExpr(AstType::VarExpr, sl), name(name) {}
  void print_name() const override { std::cout << "VarExpr\n"; }
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
};

struct TypeExpr : AstExpr {
  QualType ty;
  TypeExpr(const SourceLocation &sl, QualType ty)
      : AstExpr(AstType::TypeExpr, sl), ty(ty) {}
  TypeExpr(const SourceLocation &sl) : AstExpr(AstType::TypeExpr, sl) {}
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
};
struct FnCall : AstExpr {
  std::string name;
  std::vector<std::unique_ptr<AstExpr>> args;

  FnCall(const SourceLocation &sl, const std::string &name)
      : AstExpr(AstType::FnCall, sl), name(name){};
  FnCall(const SourceLocation &sl, const std::string &name,
         std::vector<std::unique_ptr<AstExpr>> &&args)
      : AstExpr(AstType::FnCall, sl), name(name), args(std::move(args)){};
  void print_name() const override { std::cout << "FnCall\n"; }
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
};
struct BinExpr : AstExpr {
  std::unique_ptr<AstExpr> lhs, rhs;
  Token::Type op;
  BinExpr(const SourceLocation &sl, Token::Type op,
          std::unique_ptr<AstExpr> lhs, std::unique_ptr<AstExpr> rhs)
      : AstExpr(AstType::BinExpr, sl), lhs(move(lhs)), rhs(move(rhs)), op(op){};
  void print_name() const override { std::cout << "BinExpr\n"; }    
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
};

struct RangeExpr : AstExpr {
  std::unique_ptr<ValExpr> begin, end;
  RangeExpr(const SourceLocation &sl, std::unique_ptr<ValExpr> begin,
            std::unique_ptr<ValExpr> end)
      : AstExpr(AstType::RangeExpr, sl), begin(move(begin)), end(move(end)){};
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
};

struct IfStmt : AstExpr {
  std::unique_ptr<AstExpr> condition;
  std::unique_ptr<Body> body,else_body;
  IfStmt(const SourceLocation &sl, std::unique_ptr<AstExpr> condition,std::unique_ptr<Body>&& body)
      : AstExpr(AstType::IfStmt, sl), condition(std::move(condition)),body(std::move(body)){};
  IfStmt(const SourceLocation& sl, std::unique_ptr<AstExpr> condition, std::unique_ptr<Body>&& body,std::unique_ptr<Body>&& else_body)
      : AstExpr(AstType::IfStmt, sl), condition(std::move(condition)), body(std::move(body)),else_body(std::move(else_body)) {};
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
};

struct ReturnStmt : AstExpr {
    std::unique_ptr<AstExpr> expr;
    ReturnStmt(const SourceLocation& sl, std::unique_ptr<AstExpr>&& expr) :  expr(std::move(expr)), AstExpr(AstType::ReturnStmt, sl) {}
    llvm::Value* codegen(FusionCtx& ctx) const override;
    void pretty_print() const override;
};

struct ImportExpr : AstExpr {
  std::string module;
  ImportExpr(const SourceLocation &sl, const std::string &module)
      : AstExpr(AstType::ImportExpr, sl), module(module){};
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
};

struct ClassStmt : AstExpr {
private:
    QualType get_class_type() const;
public:
    QualType ty;
    std::unique_ptr<Body> body;
    std::string_view name;
    ClassStmt(const SourceLocation& sl, std::string_view name,std::unique_ptr<Body>&& body) : AstExpr(AstType::ClassStmt, sl), body(std::move(body)),name(name), ty(get_class_type()) {};
    llvm::Value* codegen(FusionCtx& ctx) const override;
    void pretty_print() const override;
};

int precedence(Token::Type op);
class Parser {
private:
  std::vector<Token>::const_iterator it, end;
  Token pop();
  Token peek(int n = 0);
  FusionCtx &ctx;
  const FSFile &file;

public:
  Token expect(Token::Type ty, const std::string &tk);
  Parser(std::vector<Token> &tokens, FusionCtx &ctx, const FSFile &file)
      : it(tokens.begin()), end(tokens.end()), ctx(ctx), file(file){};
  std::unique_ptr<Body> parse();
  std::unique_ptr<AstExpr> parse_expr();
private:
  std::unique_ptr<AstExpr> parse_top_level();
  std::unique_ptr<FnProto> parse_fnproto();
  std::unique_ptr<FnDecl> parse_fndecl();
  std::unique_ptr<AstExpr> parse_primary();
  std::unique_ptr<FnCall> parse_fncall();
  std::unique_ptr<AstExpr> parse_binary(std::unique_ptr<AstExpr> lhs,
                                        int p = 0);
  std::unique_ptr<ValExpr> parse_valexpr();
  std::unique_ptr<VarDeclExpr> parse_var_decl();
  std::unique_ptr<VarExpr> parse_var();
  std::unique_ptr<TypeExpr> parse_type_expr();
  std::unique_ptr<IfStmt> parse_ifstmt();
  std::unique_ptr<ImportExpr> parse_import();
  std::unique_ptr<Body> parse_body();
  std::unique_ptr<ReturnStmt> parse_return();
  std::unique_ptr<ClassStmt> parse_class();
};
