#pragma once
#include "error.h"
#include "lex.h"
#include "type.h"
#include <memory>

struct FusionCtx; // insted of including "context.h" so we don't include any
                  // llvm

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
  AstType ast_type;
  SourceLocation sl;
  int indent = 0;
  AstExpr(AstType ast_type, const SourceLocation &sl) : ast_type(ast_type), sl(sl){};
  virtual llvm::Value *codegen(FusionCtx &ctx) const = 0;
  virtual void pretty_print() const = 0;
  virtual void type_check() const;
  template <typename T> T *cast() { return reinterpret_cast<T *>(this); }
  virtual bool is_expr() const = 0;
  virtual ~AstExpr() {}
};

struct Stmt : AstExpr {
  Stmt(AstType ast_type, const SourceLocation &sl);
  bool is_expr() const override;
  virtual ~Stmt(){};
};
struct Expr : AstExpr {
  QualType type;
  Expr(AstType ast_type, const SourceLocation &sl);
  Expr(AstType ast_type, const SourceLocation &sl, QualType type);
  bool is_expr() const override;
  virtual ~Expr(){};
};

struct Body : Stmt {
  std::vector<std::unique_ptr<AstExpr>> body;
  Body(const SourceLocation &sl, std::vector<std::unique_ptr<AstExpr>> body);
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
  void type_check() const override;
};
struct VarDeclExpr : Expr {
  std::string name;

  VarDeclExpr(const SourceLocation &sl, const std::string &name);
  VarDeclExpr(const SourceLocation &sl, const std::string &name, QualType &type);
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
  void type_check() const override;
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
struct FnProto : Stmt {
  std::unique_ptr<AstExpr> ret;
  std::vector<std::unique_ptr<VarDeclExpr>> args;
  std::string name;
  Linkage linkage = Linkage::Ext;
  Inline is_inline = Inline::Def;
  FnProto(const SourceLocation &sl, const std::string &name,
          std::unique_ptr<AstExpr> ret);

  FnProto(const SourceLocation &sl, const std::string &name,
          std::vector<std::unique_ptr<VarDeclExpr>> args,
          std::unique_ptr<AstExpr> ret = nullptr);

  void pretty_print() const override;
  void type_check() const override;
  llvm::Value *codegen(FusionCtx &ctx) const override;
};

struct FnDecl : Stmt {
  FnModifiers::Type mods = 0;
  std::unique_ptr<FnProto> proto;
  std::unique_ptr<Body> body;
  FnDecl(const SourceLocation &sl, std::unique_ptr<FnProto> proto,
         std::unique_ptr<Body> body, FnModifiers::Type mods = 0);
  FnDecl(const SourceLocation &sl, std::unique_ptr<FnProto> proto);
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
  void type_check() const override;
};
struct ValExpr : Expr {
  Lit val;
  ValExpr(const SourceLocation &sl, Lit val);
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
  void type_check() const override;
};
struct VarExpr : Expr {
  std::string name;

  VarExpr(const SourceLocation &sl, const std::string &name);
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
  void type_check() const override;
};

struct TypeExpr : Expr {
  TypeExpr(const SourceLocation &sl, QualType type);
  TypeExpr(const SourceLocation &sl);
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
  void type_check() const override;
};
struct FnCall : Expr {
  std::string name;
  std::vector<std::unique_ptr<AstExpr>> args;

  FnCall(const SourceLocation &sl, const std::string &name);
  FnCall(const SourceLocation &sl, const std::string &name,
         std::vector<std::unique_ptr<AstExpr>> args);
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
  void type_check() const override;
};
struct BinExpr : Expr {
  std::unique_ptr<AstExpr> lhs, rhs;
  Token::Type op;
  BinExpr(const SourceLocation &sl, Token::Type op,
          std::unique_ptr<AstExpr> lhs, std::unique_ptr<AstExpr> rhs);
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
  void type_check() const override;
};

struct RangeExpr : Expr {
  std::unique_ptr<ValExpr> begin, end;
  RangeExpr(const SourceLocation &sl, std::unique_ptr<ValExpr> begin,
            std::unique_ptr<ValExpr> end);
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
  void type_check() const override;
};

struct IfStmt : Stmt {
  std::unique_ptr<AstExpr> condition;
  std::unique_ptr<Body> body, else_body;
  IfStmt(const SourceLocation &sl, std::unique_ptr<AstExpr> condition,
         std::unique_ptr<Body> body);
  IfStmt(const SourceLocation &sl, std::unique_ptr<AstExpr> condition,
         std::unique_ptr<Body> body, std::unique_ptr<Body> else_body);
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
  void type_check() const override;
};

struct ReturnStmt : Stmt {
  std::unique_ptr<AstExpr> expr;
  ReturnStmt(const SourceLocation &sl, std::unique_ptr<AstExpr> expr);
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
  void type_check() const override;
};

struct ImportExpr : Stmt {
  std::string module;
  ImportExpr(const SourceLocation &sl, const std::string &module);
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
  void type_check() const override;
};

struct ClassStmt : Stmt {
private:

public:
  static std::unique_ptr<Type> get_class_type(const std::unique_ptr<VarExpr>& name,const std::unique_ptr<Body>& body);
  std::unique_ptr<Type> ty;
  std::unique_ptr<Body> body;
  std::unique_ptr<VarExpr> name;
  ClassStmt(const SourceLocation &sl, std::unique_ptr<VarExpr> name,
            std::unique_ptr<Body> body, std::unique_ptr<Type> ty);
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
  void type_check() const override;
};

int precedence(Token::Type op);
class Parser {
private:
  std::vector<Token>::iterator it, end;
  Token pop();
  Token peek(int n = 0);
  void consume_li();
  FusionCtx &ctx;

public:
  Token expect(Token::Type ty, const std::string &tk);
  Parser(std::vector<Token> &tokens, FusionCtx &ctx, const FSFile &file)
      : it(tokens.begin()), end(tokens.end()), ctx(ctx){};
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
  std::unique_ptr<Body> parse_class_body();
  std::unique_ptr<AstExpr> parse_inside_class();

  bool is_end_of_body();
};
Type *lookup_type(AstExpr* head,const std::string& name);