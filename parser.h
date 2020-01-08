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
        
       
};
struct FnProto : AstExpr {
    Type ret;
    std::vector<Type> args;
    ptr name;
    template<typename... Ts>
    FnProto(ptr name,Type ret, Ts... type) :
    AstExpr(AstType::FnProto),name(name),ret(ret),args({type...})  {};
};

struct FnDecl : AstExpr {
    std::unique_ptr<FnProto> proto;
    std::unique_ptr<AstExpr> body;
    FnDecl(std::unique_ptr<FnProto> prototype,std::unique_ptr<AstExpr> body) : 
    proto(move(proto)),body(move(body)),AstExpr(AstType::FnDecl){};
    
};
struct ValExpr : AstExpr {
    llvm::Constant* val;
    ValExpr(llvm::Constant* val) : AstExpr(AstType::ValExpr),val(val){}
};

struct FnCall : AstExpr {
    ptr name;
    //args
    FnCall(const ptr& name) : name(name), AstExpr(AstType::FnCall) {};
    
};
struct BinExpr : AstExpr {
    std::unique_ptr<AstExpr> lhs,rhs;
    Token::Type op;
    BinExpr(Token::Type op,std::unique_ptr<AstExpr> lhs,std::unique_ptr<AstExpr> rhs) :
    op(op),lhs(move(lhs)),rhs(move(rhs)),AstExpr(AstType::BinExpr){};
};


int pre(Token::Type op);
class Parser : Lexer {
public:
    Token expect(Token::Type ty,const std::string& tk);
    Parser(FSFile& file) : Lexer(file){};
    std::unique_ptr<FnProto> parse_fnproto();
    std::unique_ptr<FnDecl> parse_fndecl();
    std::unique_ptr<AstExpr> parse_primary();
    std::unique_ptr<FnCall> parse_fncall();
    std::unique_ptr<AstExpr> parse_binary(std::unique_ptr<AstExpr> lhs,int p=0);
    std::unique_ptr<ValExpr> parse_valexpr();
};