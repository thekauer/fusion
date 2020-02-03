#pragma once
#include "context.h"
#include "error.h"
#include "lex.h"
#include "type.h"
#include <memory>

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
};
class AstExpr {
public:
  AstType type;
  AstExpr(AstType type) : type(type){};
  virtual void print_name() { std::cout << "AstExpr\n"; };
  virtual llvm::Value *codegen(FusionCtx &ctx) = 0;
  virtual void pretty_print() = 0;
  virtual ~AstExpr() {}
  int indent = 0;
};

struct VarDeclExpr : AstExpr {
  std::string name;
  Type *ty = nullptr;
  VarDeclExpr(const std::string &name)
      : AstExpr(AstType::VarDeclExpr), name(name) {}
  VarDeclExpr(const std::string &name, Type *ty)
      : AstExpr(AstType::VarExpr), name(name), ty(ty){};
  llvm::Value *codegen(FusionCtx &ctx) override;
  void print_name() override { std::cout << "VarDeclExpr\n"; }
  void pretty_print() override;
};
struct FnProto : AstExpr {
  std::unique_ptr<AstExpr> ret;
  std::vector<std::unique_ptr<VarDeclExpr>> args;
  Token name;
  Linkage linkage = Linkage::Ext;
  Inline is_inline = Inline::Def;
  FnProto(Token name, std::unique_ptr<AstExpr> ret)
      : AstExpr(AstType::FnProto), ret(std::move(ret)), name(name){};
  FnProto(Token name,std::vector<std::unique_ptr<VarDeclExpr>>&& args,std::unique_ptr<AstExpr> ret = nullptr) :
  AstExpr(AstType::FnProto),ret(std::move(ret)),args(std::move(args)),name(name){}
  void print_name() override { std::cout << "FnProto\n"; }
  void pretty_print() override;
  llvm::Value *codegen(FusionCtx &ctx) override;
};

struct FnDecl : AstExpr {
  std::unique_ptr<FnProto> proto;
  std::vector<std::unique_ptr<AstExpr>> body;
  FnDecl(std::unique_ptr<FnProto> proto,
         std::vector<std::unique_ptr<AstExpr>> &&body)
      : AstExpr(AstType::FnDecl), proto(move(proto)), body(move(body)){};
  void print_name() override {
    std::cout << "FnDecl: " << proto->name.getName() << "\n";
  }
  llvm::Value *codegen(FusionCtx &ctx) override;
  void pretty_print() override;
};
struct ValExpr : AstExpr {
  Lit val;
  ValExpr(Lit val) : AstExpr(AstType::ValExpr), val(val) {}
  void print_name() override { std::cout << "ValExpr\n"; }
  llvm::Value *codegen(FusionCtx &ctx) override;
  void pretty_print() override;
};
struct VarExpr : AstExpr {
  std::string name;
  VarExpr(const std::string &name) : AstExpr(AstType::VarExpr), name(name) {}
  void print_name() override { std::cout << "VarExpr\n"; }
  llvm::Value *codegen(FusionCtx &ctx) override;
  void pretty_print() override;
};

struct TypeExpr : AstExpr {
  Type *ty;
  TypeExpr(Type *ty) : AstExpr(AstType::TypeExpr), ty(ty) {}
  TypeExpr() : AstExpr(AstType::TypeExpr), ty(nullptr) {}
  llvm::Value *codegen(FusionCtx &ctx) override;
  void pretty_print() override;
};
struct FnCall : AstExpr {
  std::string name;
  std::vector<std::unique_ptr<AstExpr>> args;
  FnCall(const std::string &name) : AstExpr(AstType::FnCall), name(name){};
  FnCall(const std::string &name, std::vector<std::unique_ptr<AstExpr>> &&args)
      : AstExpr(AstType::FnCall), name(name), args(std::move(args)){};
  void print_name() override { std::cout << "FnCall\n"; }
  llvm::Value *codegen(FusionCtx &ctx) override;
  void pretty_print() override;
};
struct BinExpr : AstExpr {
  std::unique_ptr<AstExpr> lhs, rhs;
  Token::Type op;
  BinExpr(Token::Type op, std::unique_ptr<AstExpr> lhs,
          std::unique_ptr<AstExpr> rhs)
      : AstExpr(AstType::BinExpr), lhs(move(lhs)), rhs(move(rhs)), op(op){};
  void print_name() override { std::cout << "BinExpr\n"; }
  llvm::Value *codegen(FusionCtx &ctx) override;
  void pretty_print() override;
};


struct RangeExpr : AstExpr {
  std::unique_ptr<ValExpr> begin,end;
  RangeExpr(std::unique_ptr<ValExpr> begin,std::unique_ptr<ValExpr> end) 
  : AstExpr(AstType::RangeExpr),begin(move(begin)),end(move(end)){};
  llvm::Value *codegen(FusionCtx& ctx) override;
  void pretty_print() override;
};

int pre(Token::Type op);
class Parser {
private:
  std::vector<Token>::const_iterator it, end;
  Token pop();
  Token peek(int n = 0);
  int indent = 0;
  FusionCtx &ctx;

public:
  int cnt = 0;
  Token expect(Token::Type ty, const std::string &tk);
  Parser(std::vector<Token> &tokens, FusionCtx &ctx)
      : it(tokens.begin()), end(tokens.end()), ctx(ctx){};
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
  std::unique_ptr<ValExpr> pop_integer();
  std::unique_ptr<RangeExpr> parse_range_expr();
  
};
