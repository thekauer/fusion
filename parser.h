#pragma once
#include "lex.h"
#include "type.h"
#include <memory>
#include "error.h"

    enum class AstType :unsigned char{
        FnDecl,
        FnCall,
        FnProto,
        BinExpr,
        ValExpr
    };
class AstExpr {
    public:
    AstType type;
    AstExpr(AstType type): type(type){};
    virtual void print_name() {std::cout <<"AstExpr\n";};
    //virtual void codegen()=0;
       
};
struct FnProto : AstExpr {
    std::unique_ptr<AstExpr> ret;
    std::vector<AstExpr> args;
    ptr name;
    FnProto(ptr name,std::unique_ptr<AstExpr> ret) :
    AstExpr(AstType::FnProto),name(name),ret(std::move(ret))  {};
    void print_name() override {std::cout << "FnProto\n";}
};

struct FnDecl : AstExpr {
    std::unique_ptr<FnProto> proto;
    std::unique_ptr<AstExpr> body;
    FnDecl(std::unique_ptr<FnProto> proto,std::unique_ptr<AstExpr> body) : 
    proto(move(proto)),body(move(body)),AstExpr(AstType::FnDecl){};
    void print_name() override {std::cout <<"FnDecl\n";}
};
struct ValExpr : AstExpr {
    llvm::Constant* val;
    ValExpr(llvm::Constant* val) : AstExpr(AstType::ValExpr),val(val){}
    void print_name()override {std::cout <<"ValExpr\n";}
};

struct FnCall : AstExpr {
    ptr name;
    //args
    FnCall(const ptr& name) : name(name), AstExpr(AstType::FnCall) {};
    void print_name() override{std::cout <<"FnCall\n";}
};
struct BinExpr : AstExpr {
    std::unique_ptr<AstExpr> lhs,rhs;
    Token::Type op;
    BinExpr(Token::Type op,std::unique_ptr<AstExpr> lhs,std::unique_ptr<AstExpr> rhs) :
    op(op),lhs(move(lhs)),rhs(move(rhs)),AstExpr(AstType::BinExpr){};
    void print_name()override {std::cout <<"BinExpr\n";}
};


int pre(Token::Type op);
class Parser{
private:
    std::vector<Token>::const_iterator it,end;
    Token pop();
    Token peek(int n=0);
public:
    Token expect(Token::Type ty,const std::string& tk);
    Parser(std::vector<Token>& tokens) : it(tokens.begin()),end(tokens.end()){};
    std::unique_ptr<FnProto> parse_fnproto();
    std::unique_ptr<FnDecl> parse_fndecl();
    std::unique_ptr<AstExpr> parse_primary();
    std::unique_ptr<FnCall> parse_fncall();
    std::unique_ptr<AstExpr> parse_binary(std::unique_ptr<AstExpr> lhs,int p=0);
    std::unique_ptr<ValExpr> parse_valexpr();
    std::unique_ptr<AstExpr> parse_expr();
};