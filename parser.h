#pragma once
#include "lex.h"
#include "type.h"
#include <memory>

class AstExpr {
    public:
    enum type_ {
        FnDecl,
        FnCall,
        FnProto
    }type;
    AstExpr(type_ type): type(type){};
        
       
};
struct FnProto : AstExpr {
    Type ret;
    std::vector<Type> args;
    template<typename... Ts>
    FnProto(Type ret, Ts... type) :AstExpr(AstExpr::FnProto),ret(ret),args({type...})  {};
};

struct FnDecl : AstExpr {
    ptr name;
    //args
    //std::unique_ptr<AstExpr> body{};
    FnDecl(const ptr& name) : name(name),AstExpr(AstExpr::FnDecl){};
    
};

struct FnCall : AstExpr {
    ptr name;
    //args
    FnCall(const ptr& name) : name(name), AstExpr(AstExpr::FnCall) {};
    
};




class Parser : Lexer {
public:
    Parser(const FSFile& file) : Lexer(file){};
    std::unique_ptr<FnProto> parse_fnproto();
};