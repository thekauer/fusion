#include "gtest/gtest.h"
#include "parser.h"
#include "context.h"
#include "source.h"
#include "lex.h"


TEST(parser,only_primary) {
    FusionCtx ctx;
    std::string code = "2";
    auto file = FSFile("test",code);
    auto lexer = Lexer(file,ctx);
    lexer.lex();
    auto tokens = lexer.tokens;
    Token::Type tks[] {Token::Lit};
    EXPECT_EQ(tokens.size(),1);
    for(int i=0; i<tokens.size(); i++) {
        EXPECT_EQ(tokens[i].type,tks[i]) <<"i: "<< i;
    }
    auto p = Parser(tokens,ctx,file);
    auto expr = p.parse_expr();
    ASSERT_NE(expr,nullptr)<<"expr is null";
    ASSERT_EQ(expr->type,AstType::ValExpr);
}

TEST(parser,primary_and_op) {
    FusionCtx ctx;
    std::string code = "2+";
    auto file = FSFile("test",code);
    auto lexer = Lexer(file,ctx);
    lexer.lex();
    auto tokens = lexer.tokens;
    Token::Type tks[] {Token::Lit,Token::Add};
    EXPECT_EQ(tokens.size(),2);
    for(int i=0; i<tokens.size(); i++) {
        EXPECT_EQ(tokens[i].type,tks[i]) <<"i: "<< i;
    }
    auto p = Parser(tokens,ctx,file);
    try {
        auto expr = p.parse_expr();
        ASSERT_EQ(expr,nullptr)<<"expr is null";
    } catch(const std::exception& e) {
        ASSERT_EQ(1,0) << "crashed";
    }
}
TEST(parser,outofbounnds) {
    FusionCtx ctx;
    std::string code = "2 + 3";
    auto file = FSFile("test",code);
    auto lexer = Lexer(file,ctx);
    lexer.lex();
    auto tokens = lexer.tokens;
    Token::Type tks[] {Token::Lit,Token::Add,Token::Lit};
    EXPECT_EQ(tokens.size(),3);
    for(int i=0; i<tokens.size(); i++) {
        EXPECT_EQ(tokens[i].type,tks[i]) <<"i: "<< i;
    }
    /*
            +
          /   \
         2     3
    */

    auto p = Parser(tokens,ctx,file);
    try {
        auto expr = p.parse_expr();
        ASSERT_NE(expr,nullptr)<<"expr is null";
        ASSERT_EQ(expr->type,AstType::BinExpr);
        auto bin = reinterpret_cast<BinExpr*>(expr.get());
        EXPECT_EQ(bin->lhs->type,AstType::ValExpr);
        EXPECT_EQ(bin->rhs->type,AstType::ValExpr);
    } catch(const std::exception& e) {
        ASSERT_EQ(1,0) << "crashed";
    }

}
TEST(parser,addition) {
    FusionCtx ctx;
    std::string code = "2 + 3 * 2\n";
    auto file = FSFile("test",code);
    auto lexer = Lexer(file,ctx);
    lexer.lex();
    auto tokens = lexer.tokens;
    EXPECT_EQ(tokens.size(),5);
    Token::Type tks[] {Token::Lit,Token::Add,Token::Lit,Token::Mul,Token::Lit};
    for(int i=0; i<tokens.size(); i++) {
        EXPECT_EQ(tokens[i].type,tks[i]) <<"i: "<< i;
    }
    /*
            *
          /   \
         +     2
        / \
       2   3
    */
    auto p = Parser(tokens,ctx,file);
    auto expr = p.parse_expr();
    ASSERT_NE(expr,nullptr)<<"expr is null";
    ASSERT_EQ(expr->type,AstType::BinExpr);
    auto bin = reinterpret_cast<BinExpr*>(expr.get());

    EXPECT_EQ(bin->lhs->type,AstType::ValExpr);

    ASSERT_EQ(bin->rhs->type,AstType::BinExpr);
    auto r = reinterpret_cast<BinExpr*>(bin->rhs.get());
    EXPECT_EQ(r->lhs->type,AstType::ValExpr);
    EXPECT_EQ(r->rhs->type,AstType::ValExpr);

}
TEST(parser,binexpr) {
    FusionCtx ctx;
    std::string code = "5 = 3 + 2";
    auto file = FSFile("test",code);
    auto lexer = Lexer(file,ctx);
    lexer.lex();
    auto tokens = lexer.tokens;
    EXPECT_EQ(tokens.size(),5);
    Token::Type tks[] {Token::Lit,Token::Eq,Token::Lit,Token::Add,Token::Lit};
    for(int i=0; i<tokens.size(); i++) {
        EXPECT_EQ(tokens[i].type,tks[i]) <<"i: "<< i;
    }
    /*
            =
          /   \
         5     +
             /   \
            3     2
    */
    auto p = Parser(tokens,ctx,file);
    auto expr = p.parse_expr();
    ASSERT_NE(expr,nullptr)<<"expr is null";
    ASSERT_EQ(expr->type,AstType::BinExpr);
    auto bin = reinterpret_cast<BinExpr*>(expr.get());
    EXPECT_EQ(bin->lhs->type,AstType::ValExpr) << "left should be varexpr (a=)";
    ASSERT_EQ(bin->rhs->type,AstType::BinExpr)<<"right side should be 3+2";
    /*
    auto r = reinterpret_cast<BinExpr*>(bin->rhs.get());
    EXPECT_EQ(r->op,Token::Add) << "Expected a +";
    EXPECT_EQ(r->lhs->type,AstType::ValExpr) << "3 should be a val";
    EXPECT_EQ(r->rhs->type,AstType::ValExpr) << "2 should be a type";
    */

}