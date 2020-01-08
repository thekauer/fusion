#include "parser.h"

Token Parser::expect(Token::Type ty,const std::string& tk) {
    auto t = next();
    if(t.type!=ty) error(Error_e::ExpectedToken,"Expected a " + tk,
    sl_cast(this));
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

    auto t = next();
    if(t.type!=Token::Kw)return nullptr;
    auto name = expect(Token::Id, "identifier");
    //generics
    expect(Token::Lp,"(");
    //args
    expect(Token::Rp,")");
    //MAybe return type
    auto ret_ty =Type({IntegralType(I32,true,Copy)});
    return std::make_unique<FnProto>(name.hash,ret_ty);   
}

std::unique_ptr<FnDecl> Parser::parse_fndecl() {
    auto proto = parse_fnproto();
    if(!proto) return nullptr;
    auto t =expect(Token::Gi,"greater indentation");
    auto fn_indent =indent;
    auto body = move(parse_primary());
    return std::make_unique<FnDecl>(move(proto),move(body));
}

std::unique_ptr<ValExpr> Parser::parse_valexpr() {
    auto t = next();
    if(t.type==Token::Lit) {
        return std::make_unique<ValExpr>(t.value.val);
    }
    return nullptr;
}

std::unique_ptr<AstExpr> Parser::parse_primary() {
    std::unique_ptr<AstExpr> expr = parse_valexpr();
    if(expr)return expr;
    return parse_fncall();
}
std::unique_ptr<AstExpr> Parser::parse_binary(std::unique_ptr<AstExpr> lhs,int p){
    if(it==end)return lhs;
    auto op=next().type;
    auto tp=pre(op);
    auto rhs=parse_primary();
    if(it==end) {
        return std::make_unique<BinExpr>(op,move(lhs),move(rhs));
    }
    auto np=pre(next().type);//peek
    it--;
    if(tp>=np) {
        return parse_binary(std::make_unique<BinExpr>(op,move(lhs),move(rhs)));
    }

    return std::make_unique<BinExpr>(op,move(lhs),move(parse_binary(move(rhs))));
}

std::unique_ptr<FnCall> Parser::parse_fncall() {
    auto name = next();
    if(name.type!=Token::Id) return nullptr;
    auto fn_name=name.hash;
    expect(Token::Lp,"(");
    //args
    std::vector<AstExpr> args;
    auto arg = move(parse_primary());
    expect(Token::Rp,")");
}