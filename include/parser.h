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
  IfExpr,
  ImportExpr,
  ModExpr,
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
  std::vector<std::unique_ptr<AstExpr>> body{};
  FnDecl(const SourceLocation &sl, std::unique_ptr<FnProto> proto,
         std::vector<std::unique_ptr<AstExpr>> &&body,
         FnModifiers::Type mods = 0)
      : AstExpr(AstType::FnDecl, sl), proto(move(proto)), body(move(body)),
        mods(mods){};
  FnDecl(const SourceLocation &sl, std::unique_ptr<FnProto> proto)
      : AstExpr(AstType::FnDecl, sl), proto(move(proto)),
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

struct IfExpr : AstExpr {
  std::unique_ptr<AstExpr> condition;
  IfExpr(const SourceLocation &sl, std::unique_ptr<AstExpr> condition)
      : AstExpr(AstType::IfExpr, sl), condition(std::move(condition)){};
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
};

struct ImportExpr : AstExpr {
  std::string module;
  ImportExpr(const SourceLocation &sl, const std::string &module)
      : AstExpr(AstType::ImportExpr, sl), module(module){};
  llvm::Value *codegen(FusionCtx &ctx) const override;
  void pretty_print() const override;
};

int precedence(Token::Type op);
class Parser {
private:
  std::vector<Token>::const_iterator it, end;
  Token pop();
  Token peek(int n = 0);
  int indent = 0;
  FusionCtx &ctx;
  const FSFile &file;

public:
  int cnt = 0;
  Token expect(Token::Type ty, const std::string &tk);
  Parser(std::vector<Token> &tokens, FusionCtx &ctx, const FSFile &file)
      : it(tokens.begin()), end(tokens.end()), ctx(ctx), file(file){};
  std::unique_ptr<FnProto> parse_fnproto();
  std::unique_ptr<FnDecl> parse_fndecl();
  std::unique_ptr<AstExpr> parse_primary();
  std::unique_ptr<FnCall> parse_fncall();
  std::unique_ptr<AstExpr> parse_binary(std::unique_ptr<AstExpr> lhs,
                                        int p = 0);
  std::unique_ptr<ValExpr> parse_valexpr();
  std::unique_ptr<AstExpr> parse_expr();
  std::unique_ptr<AstExpr> parse_infered_var_decl(const std::string &name);
  std::unique_ptr<AstExpr> parse_var_decl();
  std::unique_ptr<VarDeclExpr> parse_arg();
  std::unique_ptr<VarExpr> parse_var();
  std::unique_ptr<TypeExpr> parse_type_expr();
  std::unique_ptr<IfExpr> parse_if_expr();
  std::unique_ptr<ImportExpr> parse_import();
};
