#include "parser.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include <csignal>

Token Parser::pop() {
    if(it==end)return Token(Token::Eof,(--it)->sl);
    return *it++;
}
Token Parser::peek(int n) {
    return *(it + n);
}

Token Parser::expect(Token::Type ty, const std::string &tk) {
    auto t = pop();
    if (t.type != ty)
        serror(Error_e::ExpectedToken, "Expected a(n) " + tk /*, t.sl*/);
    return t;
}

int pre(Token::Type op) {
    switch (op) {
    case Token::Eq:
        return 1;
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
    if (t.type != Token::Kw)
        return nullptr;
    pop();
    auto name = expect(Token::Id, "identifier");
    // generics
    expect(Token::Lp, "(");
    // args
    expect(Token::Rp, ")");
    // MAybe return type
    return std::make_unique<FnProto>(name, nullptr);
}

std::unique_ptr<FnDecl> Parser::parse_fndecl() {
    auto fn_indent = peek().sl.indent;
    auto proto = parse_fnproto();
    if (!proto)
        return nullptr;
    expect(Token::Gi, "greater indentation");
    ++indent;

    std::vector<std::unique_ptr<AstExpr>> body;
    auto expr = parse_expr();
    if (!expr)
        serror(Error_e::EmptyFnBody, "Empty function body" /*, peek().sl*/);
    while (expr) {
        body.push_back(std::move(expr));

        if (peek().sl.indent <= fn_indent) {
            break;
        }
        if (peek().type == Token::Gi) {
            pop();
            ++indent;
        }
        if (peek().type == Token::Li) {
            pop();
            --indent;
        }
        if (peek().type == Token::N) {
            pop();
        }

        expr = parse_expr();
    }
    auto ret = std::make_unique<FnDecl>(move(proto), move(body));
    return ret;
}

std::unique_ptr<ValExpr> Parser::parse_valexpr() {
    auto t = peek();
    if (t.type == Token::Lit) {
        pop();
        return std::make_unique<ValExpr>(t.getValue());
    }
    return nullptr;
}

std::unique_ptr<AstExpr> Parser::parse_primary() {
    if (peek().type == Token::N)
        pop();
    std::unique_ptr<AstExpr> expr = parse_valexpr();
    if (expr)
        return expr;
    expr = parse_var_decl();
    if (expr)
        return expr;
    expr = parse_fncall();
    if (expr)
        return expr;
    return parse_var();
}
std::unique_ptr<AstExpr> Parser::parse_binary(std::unique_ptr<AstExpr> lhs,
        int p) {
    if (!lhs)
        return nullptr;
    if (it == end)
        return lhs;
    auto op = peek().type;
    // check if op is actually an operator
    if (pre(op) == -1)
        return lhs;
    pop();
    auto tp = pre(op);
    auto rhs = parse_primary();
    if (it == end) {
        return std::make_unique<BinExpr>(op, move(lhs), move(rhs));
    }
    auto np = pre(peek(1).type); // peek
    if (np == -1) {
        return std::make_unique<BinExpr>(op, move(lhs), move(rhs));
    }
    if (tp >= np) {
        return parse_binary(std::make_unique<BinExpr>(op, move(lhs), move(rhs)));
    }

    return std::make_unique<BinExpr>(op, move(lhs), parse_binary(move(rhs)));
}

std::unique_ptr<TypeExpr> Parser::parse_type_expr() {
    if (peek().type != Token::Kw) {
        return nullptr;
    }
    switch (pop().getKw()) {
    case Kw_e::I32:
        return std::make_unique<TypeExpr>(ctx.getI32());
    case Kw_e::I8:
        return std::make_unique<TypeExpr>(ctx.getI8());
    case Kw_e::I16:
        return std::make_unique<TypeExpr>(ctx.getI16());
    case Kw_e::I64:
        return std::make_unique<TypeExpr>(ctx.getI64());
    case String:
        return std::make_unique<TypeExpr>(ctx.getString());
    default:
        return nullptr;
    }
    return nullptr;
}

std::unique_ptr<VarDeclExpr> Parser::parse_var_decl() {
    if (!(peek().type == Token::Id && peek(1).type == Token::DoubleDot)) {
        return nullptr;
    }
    auto id = pop();
    pop(); // pop Double dot
    std::unique_ptr<TypeExpr> ty = parse_type_expr();
    if (!ty) {
        // error expected type expr
        return nullptr; // return Infer type
    }
    return std::make_unique<VarDeclExpr>(id.getName(), ty->ty);
}

std::unique_ptr<AstExpr> Parser::parse_expr() {
    auto lhs = parse_primary();
    if (!lhs)
        return parse_binary(std::move(lhs));
}

std::unique_ptr<FnCall> Parser::parse_fncall() {
    auto name = peek();
    if (name.type != Token::Id)
        return nullptr;
    if (peek(1).type != Token::Lp) {
        return nullptr;
    }
    pop(); // pop name
    pop(); // pop (

    // args
    std::vector<std::unique_ptr<AstExpr>> args;
    auto arg = parse_expr();
    args.push_back(std::move(arg));
    expect(Token::Rp, ")");
    return std::make_unique<FnCall>(name.getName(), std::move(args));
}

std::unique_ptr<VarExpr> Parser::parse_var() {
    auto name = peek();
    if (name.type != Token::Id) {
        return nullptr;
    }
    pop();

    return std::make_unique<VarExpr>(name.getName());
}

void FnProto::pretty_print() {
    llvm::outs() << "fn " << name.getName() << "(";
    for (const auto &arg : args) {
        arg->pretty_print();
        llvm::outs() << ",";
    }
    if (args.size() == 0)
        llvm::outs() << "(";
    llvm::outs() << "\b)";
}

void FnDecl::pretty_print() {
    proto->pretty_print();
    llvm::outs() << "\n";
    for (const auto &b : body) {
        llvm::outs() << " ";
        b->pretty_print();
    }
}

void ValExpr::pretty_print() {
    llvm::outs() << "val";
}

void VarDeclExpr::pretty_print() {
    ty->print(llvm::outs() << name << " : ");
    llvm::outs() << "\n";
}
void VarExpr::pretty_print() {
    llvm::outs() << name;
}
void TypeExpr::pretty_print() {
    ty->print(llvm::outs());
}
void FnCall::pretty_print() {
    llvm::outs() << name << "(";
    for (const auto &arg : args) {
        arg->pretty_print();
        llvm::outs() << ",";
    }
    if (args.size() == 0)
        llvm::outs() << "(";
    llvm::outs() << "\b)\n";
}

void BinExpr::pretty_print() {
    lhs->pretty_print();
    llvm::outs() << " op ";
    rhs->pretty_print();
}
