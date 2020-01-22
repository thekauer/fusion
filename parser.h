#pragma once
#include "lex.h"
#include "context.h"
#include <memory>
#include "error.h"

enum class AstType :unsigned char {
    FnDecl,
    FnCall,
    FnProto,
    BinExpr,
    ValExpr,
    TypeExpr,
    VarExpr
};
class AstExpr {
public:
    AstType type;
    AstExpr(AstType type): type(type) {};
    virtual void print_name() {
        std::cout <<"AstExpr\n";
    };
    virtual llvm::Value* codegen(FusionCtx& ctx)=0;
    virtual ~AstExpr() {}

};
struct FnProto : AstExpr {
    std::unique_ptr<AstExpr> ret;
    std::vector<std::unique_ptr<AstExpr>> args;
    Token name;
    Linkage linkage =Linkage::Ext;
    Inline is_inline = Inline::Def;
    FnProto(Token name,std::unique_ptr<AstExpr> ret) :
        AstExpr(AstType::FnProto),name(name),ret(std::move(ret))  {};
    void print_name() override {
        std::cout << "FnProto\n";
    }
    llvm::Value* codegen(FusionCtx& ctx) override;
};

struct FnDecl : AstExpr {
    std::unique_ptr<FnProto> proto;
    std::unique_ptr<AstExpr> body;
    FnDecl(std::unique_ptr<FnProto> proto,std::unique_ptr<AstExpr> body) :
        proto(move(proto)),body(move(body)),AstExpr(AstType::FnDecl) {};
    void print_name() override {
        std::cout <<"FnDecl\n";
    }
    llvm::Value* codegen(FusionCtx& ctx) override;
};
struct ValExpr : AstExpr {
    llvm::Constant* val;
    ValExpr(llvm::Constant* val) : AstExpr(AstType::ValExpr),val(val) {}
    void print_name()override {
        std::cout <<"ValExpr\n";
    }
    llvm::Value* codegen(FusionCtx& ctx) override;
};

struct VarExpr : AstExpr {
    std::string name;
    VarExpr(std::string name) : AstExpr(AstType::VarExpr),name(name) {}
    llvm::Value* codegen(FusionCtx& ctx) override;

};

struct TypeExpr : AstExpr {
    llvm::Type* type;
    TypeExpr(llvm::Type* type) : AstExpr(AstType::TypeExpr),type(type) {}
    llvm::Value* codegen(FusionCtx& ctx) override;
};
struct FnCall : AstExpr {
    Token name;
    //args
    std::vector<std::unique_ptr<AstExpr>> args;
    FnCall(Token name) : name(name), AstExpr(AstType::FnCall) {};
    void print_name() override {
        std::cout <<"FnCall\n";
    }
    llvm::Value* codegen(FusionCtx& ctx) override;
};
struct BinExpr : AstExpr {
    std::unique_ptr<AstExpr> lhs,rhs;
    Token::Type op;
    BinExpr(Token::Type op,std::unique_ptr<AstExpr> lhs,std::unique_ptr<AstExpr> rhs) :
        op(op),lhs(move(lhs)),rhs(move(rhs)),AstExpr(AstType::BinExpr) {};
    void print_name()override {
        std::cout <<"BinExpr\n";
    }
    llvm::Value* codegen(FusionCtx& ctx) override;
};


int pre(Token::Type op);
class Parser {
private:
    std::vector<Token>::const_iterator it,end;
    Token pop();
    Token peek(int n=0);
public:
    Token expect(Token::Type ty,const std::string& tk);
    Parser(std::vector<Token>& tokens) : it(tokens.begin()),end(tokens.end()) {};
    std::unique_ptr<FnProto> parse_fnproto();
    std::unique_ptr<FnDecl> parse_fndecl();
    std::unique_ptr<AstExpr> parse_primary();
    std::unique_ptr<FnCall> parse_fncall();
    std::unique_ptr<AstExpr> parse_binary(std::unique_ptr<AstExpr> lhs,int p=0);
    std::unique_ptr<ValExpr> parse_valexpr();
    std::unique_ptr<AstExpr> parse_expr();
};
