#include "parser.h"



Token Parser::pop() {
    return *it++;
}
Token Parser::peek(int n) {
    return *(it+n);
}

Token Parser::expect(Token::Type ty,const std::string& tk) {
    auto t = pop();
    if(t.type!=ty) error(Error_e::ExpectedToken,"Expected a(n) " + tk,
    t.sl);
    return t;
}

int pre(Token::Type op) {
    switch (op)
    {
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


std::unique_ptr<FnProto> Parser::parse_fnproto() {    


    auto t = peek();
    if(t.type!=Token::Kw)return nullptr;
    pop();
    auto name = expect(Token::Id, "identifier");
    
    //generics
    expect(Token::Lp,"(");
    //args
    expect(Token::Rp,")");
    //MAybe return type
    return std::make_unique<FnProto>(name.str,nullptr);   
}

std::unique_ptr<FnDecl> Parser::parse_fndecl() {
    auto proto = parse_fnproto();
    if(!proto) return nullptr;
    auto t =expect(Token::Gi,"greater indentation");

    auto body = parse_primary();
    if(!body)error(Error_e::EmptyFnBody,"Empty function body",peek().sl);
    return std::make_unique<FnDecl>(move(proto),move(body));
}

std::unique_ptr<ValExpr> Parser::parse_valexpr() {
    auto t = peek();
    if(t.type==Token::Lit) {
        pop();
        return std::make_unique<ValExpr>(t.value);
    }
    return nullptr;
}

std::unique_ptr<AstExpr> Parser::parse_primary() {
    std::unique_ptr<AstExpr> expr = parse_valexpr();
    if(expr)return expr;
    return parse_fncall();
}
std::unique_ptr<AstExpr> Parser::parse_binary(std::unique_ptr<AstExpr> lhs,int p){
    if(!lhs)return nullptr;
    if(it==end)return lhs;
    auto op=peek().type;
    //check if op is actually an operator
    pop();
    auto tp=pre(op);
    auto rhs=parse_primary();
    if(it==end) {
        return std::make_unique<BinExpr>(op,move(lhs),move(rhs));
    }
    auto np=pre(peek(1).type);//peek
    if(np==-1) {
        return std::make_unique<BinExpr>(op,move(lhs),move(rhs));
    }
    if(tp>=np) {
        return parse_binary(std::make_unique<BinExpr>(op,move(lhs),move(rhs)));
    }

    return std::make_unique<BinExpr>(op,move(lhs),move(parse_binary(move(rhs))));
}

std::unique_ptr<AstExpr> Parser::parse_expr() {
    return parse_binary(std::move(parse_primary()));
}

std::unique_ptr<FnCall> Parser::parse_fncall() {
    auto name = peek();
    if(name.type!=Token::Id) return nullptr;
    pop();

    auto fn_name=name.str;
    expect(Token::Lp,"(");
    //args
    std::vector<AstExpr> args;
    auto arg = parse_expr();
    expect(Token::Rp,")");
    return std::make_unique<FnCall>(fn_name);
}


