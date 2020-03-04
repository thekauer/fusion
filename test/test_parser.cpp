#include "gtest/gtest.h"
#include "parser.h"
#include "context.h"
#include "source.h"
#include "lex.h"

//use a test suite
TEST(parser,binexpr) {
    FusionCtx ctx;
    std::string code = "a = 3 + 2";
    auto file = FSFile("test",code);
    auto lexer = Lexer(file,ctx);
    lexer.lex();
    auto tokens = lexer.tokens;
    EXPECT_EQ(tokens.size(),5);
    Token::Type tks[] {Token::Id,Token::Eq,Token::Lit,Token::Add,Token::Lit};
    for(int i=0; i<tokens.size(); i++) {
        EXPECT_EQ(tokens[i].type,tks[i]) <<"i: "<< i;
    }
    auto p = Parser(tokens,ctx);
    auto expr = p.parse_expr();
    ASSERT_NE(expr,nullptr)<<"expr is null";
    ASSERT_EQ(expr->type,AstType::BinExpr);
    auto bin = reinterpret_cast<BinExpr*>(expr.get());
    EXPECT_EQ(bin->lhs->type,AstType::VarExpr) << "left should be varexpr (a=)";
    ASSERT_EQ(bin->rhs->type,AstType::BinExpr)<<"right side should be 3+2";
    /*
    auto r = reinterpret_cast<BinExpr*>(bin->rhs.get());
    EXPECT_EQ(r->op,Token::Add) << "Expected a +";
    EXPECT_EQ(r->lhs->type,AstType::ValExpr) << "3 should be a val";
    EXPECT_EQ(r->rhs->type,AstType::ValExpr) << "2 should be a type";
    */

}