#include "parser.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include <csignal>

Token Parser::pop() {
    if (it == end)
        return Token(Token::Eof, (it-1)->sl);
    return *it++;
}
Token Parser::peek(int n) {
    return *(it + n);
}

Token Parser::expect(Token::Type ty, const std::string &tk) {
    auto t = pop();
    if (t.type != ty)
        Error::ExpectedToken(file,t.sl,"Expected " + tk);
    return t;
}

int precedence(Token::Type op) {
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

    if (peek().type != Token::Kw && peek().getKw() != Kw_e::Fn)

        return nullptr;
    pop();
    auto namet = pop();
    if (namet.type != Token::Id) {
        serror(Error_e::Unk, "NAMET-FNPROTO");
    }
    auto name = namet.getName();
    // generics
    expect(Token::Lp, "a (");
    // args
    auto arg = parse_arg();
    std::vector<std::unique_ptr<VarDeclExpr>> args;
    while (arg) {
        args.push_back(std::move(arg));

        if (peek().type == Token::Comma) {
            pop();
        }

        else {
            if (peek().type != Token::Rp) {
                serror(Error_e::Unk, "expected , or )");
            }
        }
        arg = parse_arg();
    }

    expect(Token::Rp, "a )");
    // MAybe return type
    return std::make_unique<FnProto>(namet.sl, name, std::move(args));
}

std::unique_ptr<FnDecl> Parser::parse_fndecl() {
    FnModifiers::Type mods = 0;
    if (peek().type == Token::Kw && peek().getKw() == Extern) {
        pop();
        mods |= FnModifiers::Extern;
    }

    auto fn_indent = peek().sl.indent;
    auto proto = parse_fnproto();
    if (!proto)
        return nullptr;
    if (mods & FnModifiers::Extern) {
        if (peek().type != Token::N) {
            serror(Error_e::Unk, "should be new line");
        }
        return std::make_unique<FnDecl>(peek().sl, move(proto));
    }
    expect(Token::Gi, "greater indentation");
    ++indent;

    std::vector<std::unique_ptr<AstExpr>> body;
    auto expr = parse_expr();
    if (!expr)
        Error::EmptyFnBody(file,peek().sl);
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
    auto ret = std::make_unique<FnDecl>(proto->sl, move(proto), move(body), mods);
    return ret;
}

std::unique_ptr<ValExpr> Parser::parse_valexpr() {
    auto t = peek();
    if (t.type == Token::Lit) {
        pop();
        return std::make_unique<ValExpr>(t.sl, t.getValue());
    }
    if (t.type == Token::Kw) {
        if (t.getKw() == Kw_e::True) {
            pop();
            return std::make_unique<ValExpr>(
                       t.sl, Lit(true));
        }
        if (t.getKw() == Kw_e::False) {
            pop();
            return std::make_unique<ValExpr>(
                       t.sl, Lit(false));
        }
    }
    return nullptr;
}

std::unique_ptr<AstExpr> Parser::parse_primary() {
    if (peek().type == Token::N)
        pop();

    std::unique_ptr<AstExpr> expr = parse_valexpr();
    if (expr)
        return expr;
    expr = parse_fncall();
    if (expr)
        return expr;
    expr = parse_var_decl();
    if (expr)
        return expr;
    return parse_var();
}
std::unique_ptr<AstExpr> Parser::parse_binary(std::unique_ptr<AstExpr> lhs,
        int p) {
    /*
    if (!lhs)
      return nullptr;
    if (it == end)
      return lhs;
    auto op = peek().type;
    auto loc = peek().sl;
    // check if op is actually an operator
    if (pre(op) == -1)
      return lhs;
    pop();
    auto tp = pre(op);
    auto rhs = parse_primary();
    if (it == end) {
      return std::make_unique<BinExpr>(loc, op, move(lhs), move(rhs));
    }
    auto np = pre(peek(1).type); // peek
    if (np == -1) {
      return std::make_unique<BinExpr>(loc, op, move(lhs), move(rhs));
    }
    if (tp >= np) {
      return parse_binary(
          std::make_unique<BinExpr>(loc, op, move(lhs), move(rhs)));
    }

    return std::make_unique<BinExpr>(loc, op, move(lhs), parse_binary(move(rhs)));
    */
    while(it!=end) {
        auto op = peek().type;
        auto loc = peek().sl;
        auto TokPrec = precedence(op);
        if(TokPrec < p) {
            return lhs;
        }

        pop(); //pop the op
        auto rhs = parse_primary();
        if(!rhs) {
            return nullptr;
        }
        if(it==end) {
            return std::make_unique<BinExpr>(loc,op,std::move(lhs),std::move(rhs));
        }
        auto next_token=peek().type;
        auto NextPrec = precedence(next_token);
        if(TokPrec<NextPrec) {
            rhs = parse_binary(std::move(rhs),TokPrec+1);
            if(!rhs) {
                return nullptr;
            }
        }

        lhs = std::make_unique<BinExpr>(loc,op,std::move(lhs),std::move(rhs));
    }
    return lhs;
}

std::unique_ptr<TypeExpr> Parser::parse_type_expr() {
    //implement references and options


    //eh
    if (peek().type != Token::Kw) {
        return nullptr;
    }
    auto sl = peek().sl;
    switch (pop().getKw()) {
    case Kw_e::I32:
        return std::make_unique<TypeExpr>(sl, QualType(Type::get_i32()));
    case Kw_e::I8:
        return std::make_unique<TypeExpr>(sl, QualType(Type::get_i8()));
    case Kw_e::I16:
        return std::make_unique<TypeExpr>(sl, QualType(Type::get_i16()));
    case Kw_e::I64:
        return std::make_unique<TypeExpr>(sl, QualType(Type::get_i64()));
    case Kw_e::Bool:
        return std::make_unique<TypeExpr>(sl, QualType(Type::get_bool()));
    case Kw_e::Drop:
        return std::make_unique<TypeExpr>(sl);
    default:
        return nullptr;
    }
    return nullptr;
}

std::unique_ptr<AstExpr>
Parser::parse_infered_var_decl(const std::string &name) {
    if (peek().type != Token::Eq) {
        serror(Error_e::Unk, "You must assign a value to an infered type decl.");
    } else {
        pop(); // pop the =
        auto val = parse_valexpr();
        if (!val) {
            serror(Error_e::Unk, "expected a literal");
        }

        auto lhs = std::make_unique<VarDeclExpr>(peek().sl, name, val->val.ty);
        return std::make_unique<BinExpr>(peek().sl, Token::Eq, std::move(lhs),
                                         std::move(val));
    }
    return nullptr;
}

std::unique_ptr<VarDeclExpr> Parser::parse_arg() {
    auto ty_arg = parse_type_expr();
    if (ty_arg) {
        return std::make_unique<VarDeclExpr>(peek().sl, "", ty_arg->ty);
    }

    if (peek().type == Token::Id) {
        std::string id = pop().getName();
        if (peek().type == Token::DoubleDot) {
            pop();
            auto ty = parse_type_expr();
            if (ty) {

                return std::make_unique<VarDeclExpr>(peek().sl, id, ty->ty);
            } else {
                serror(Error_e::Unk, "invalid argument type");
            }
        }
    }

    if (peek().type == Token::Rp) {
        return nullptr;
    }
    /*
    auto id = peek();
    if (peek().type != Token::Id)
      return nullptr;
    pop();
    auto t = peek();
    if (t.type == Token::Comma || t.type == Token::Rp) {
      llvm::outs() << "Vardecl made here";
      return std::make_unique<VarDeclExpr>(id.getName());
    }

    if(t.type==Token::DoubleDot) {
      llvm::outs() << "DOUBLE DOT\n";
      pop();
      auto ty = parse_type_expr();
      if (!ty) {
        serror(Error_e::Unk, "Unknown type");
      }

      return std::make_unique<VarDeclExpr>(id.getName(),ty->ty);
    }*/
    serror(Error_e::Unk, "Parse arg unreachable");
}

std::unique_ptr<AstExpr> Parser::parse_var_decl() {
    if (peek().type != Token::Id || peek(1).type != Token::DoubleDot) {
        return nullptr;
    }
    auto id = pop();
    pop(); //DoubleDot
    std::unique_ptr<TypeExpr> ty = parse_type_expr();
    if(!ty) {
        serror(Error_e::Unk,"Expected type expr");
    }
    return std::make_unique<VarDeclExpr>(peek().sl, id.getName(), ty->ty);
}

std::unique_ptr<AstExpr> Parser::parse_expr() {
    auto lhs = parse_primary();
    if (lhs) // fix this
        return parse_binary(std::move(lhs));
    return lhs;
}

std::unique_ptr<FnCall> Parser::parse_fncall() {
    auto namet = peek();
    if (namet.type != Token::Id)
        return nullptr;
    if (peek(1).type != Token::Lp) {
        return nullptr;
    }
    pop(); // pop name
    pop(); // pop (
    auto name = namet.getName();

    // args
    std::vector<std::unique_ptr<AstExpr>> args;
    auto arg = parse_expr();
    args.push_back(std::move(arg));
    while(peek().type==Token::Comma) {
        pop();
        auto a = parse_expr();
        args.push_back(std::move(a));
    }
    expect(Token::Rp, "a )");
    return std::make_unique<FnCall>(namet.sl, name, std::move(args));
}

std::unique_ptr<VarExpr> Parser::parse_var() {
    auto name = peek();
    if (name.type != Token::Id) {
        return nullptr;
    }
    pop();


    return std::make_unique<VarExpr>(name.sl, name.getName());
}



std::unique_ptr<IfExpr> Parser::parse_if_expr() {
    if (peek().type == Token::Kw && peek().getKw() == Kw_e::If) {
        pop(); // pop if
        auto ret = std::make_unique<IfExpr>(peek().sl, parse_expr());
        // parse fn body
    }
    return nullptr;
}

std::unique_ptr<ImportExpr> Parser::parse_import() {
    if (peek().type != Token::Kw)
        return nullptr;
    if (peek().getKw() != Kw_e::Import)
        return nullptr;
    pop();
    if (peek().type == Token::Id) {
        auto loc = peek().sl;
        return std::make_unique<ImportExpr>(loc, pop().getName());
    } // else kéne egy id error
    if (pop().type != Token::N) {
        serror(Error_e::Unk, "expected a new line");
    }
    return nullptr;
}

void FnProto::pretty_print() const {
    llvm::outs() << "fn " << name << "(";
    for (const auto &arg : args) {
        arg->pretty_print();
        llvm::outs() << ",";
    }
    if (args.size() == 0)
        llvm::outs() << "(";
    llvm::outs() << "\b)";
}

void FnDecl::pretty_print() const {
    proto->pretty_print();
    llvm::outs() << "\n";
    for (const auto &b : body) {
        llvm::outs() << " ";
        b->pretty_print();
        llvm::outs() << "\n";
    }
    llvm::outs() << "\n";
}

void ValExpr::pretty_print() const {
    llvm::outs() << "val";
}

void VarDeclExpr::pretty_print() const {
    llvm::outs() << name << " : " << ty.get_type_ptr()->get_name().data();
}
void VarExpr::pretty_print() const {
    llvm::outs() << name;
}
void TypeExpr::pretty_print() const {
    llvm::outs() << ty.get_type_ptr()->get_name().data();
}
void FnCall::pretty_print() const {
    llvm::outs() << name << "(";
    for (const auto &arg : args) {
        arg->pretty_print();
        llvm::outs() << ",";
    }
    if (args.size() == 0)
        llvm::outs() << "(";
    llvm::outs() << "\b)\n";
}

void BinExpr::pretty_print() const {
    lhs->pretty_print();
    switch (op) {
    case Token::Add:
        llvm::outs() << "+";
        break;
    case Token::Eq:
        llvm::outs() << "=";
        break;
    case Token::Mul:
        llvm::outs() << "*";
        break;
    default:
        llvm::outs() << " op ";
        break;
    }
    rhs->pretty_print();
}

void RangeExpr::pretty_print() const {
    if (begin)
        begin->pretty_print();
    llvm::outs() << "..";
    if (end)
        end->pretty_print();
}

void IfExpr::pretty_print() const {
    llvm::outs() << "if ";
    condition->pretty_print();
}

void ImportExpr::pretty_print() const {
    llvm::outs() << "import " << module;
}
