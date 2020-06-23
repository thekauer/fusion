#include "context.h"
#include "lex.h"
#include "source.h"
#include "gtest/gtest.h"

TEST(Lex, Keywords) {
    // test _
    std::string code = "fn for i8 i16 i32 i64 string if import export extern mod "
                       "true false bool";
    auto file = FSFile("", code);
    Lexer l = Lexer(file);
    auto t = l.next();
    EXPECT_EQ(t.type, Token::Kw);
    EXPECT_EQ(t.getKw(), Kw_e::Fn);
    t = l.next();
    EXPECT_EQ(t.type, Token::Kw);
    EXPECT_EQ(t.getKw(), Kw_e::For);
    t = l.next();
    EXPECT_EQ(t.type, Token::Kw);
    EXPECT_EQ(t.getKw(), Kw_e::I8);
    t = l.next();
    EXPECT_EQ(t.type, Token::Kw);
    EXPECT_EQ(t.getKw(), Kw_e::I16);
    t = l.next();
    EXPECT_EQ(t.type, Token::Kw);
    EXPECT_EQ(t.getKw(), Kw_e::I32);
    t = l.next();
    EXPECT_EQ(t.type, Token::Kw);
    EXPECT_EQ(t.getKw(), Kw_e::I64);
    t = l.next();
    EXPECT_EQ(t.type, Token::Kw);
    EXPECT_EQ(t.getKw(), Kw_e::String);
    t = l.next();
    EXPECT_EQ(t.type, Token::Kw);
    EXPECT_EQ(t.getKw(), Kw_e::If);
    t = l.next();
    EXPECT_EQ(t.type, Token::Kw);
    EXPECT_EQ(t.getKw(), Kw_e::Import);
    t = l.next();
    EXPECT_EQ(t.type, Token::Kw);
    EXPECT_EQ(t.getKw(), Kw_e::Export);
    t = l.next();
    EXPECT_EQ(t.type, Token::Kw);
    EXPECT_EQ(t.getKw(), Kw_e::Extern);
    t = l.next();
    EXPECT_EQ(t.type, Token::Kw);
    EXPECT_EQ(t.getKw(), Kw_e::Module);
    t = l.next();
    EXPECT_EQ(t.type, Token::Kw);
    EXPECT_EQ(t.getKw(), Kw_e::True);
    t = l.next();
    EXPECT_EQ(t.type, Token::Kw);
    EXPECT_EQ(t.getKw(), Kw_e::False);
    t = l.next();
    EXPECT_EQ(t.type, Token::Kw);
    EXPECT_EQ(t.getKw(), Kw_e::Bool);
}

TEST(Lex, Operator) {
    std::string code = "+ - * / % ^ ! = & | ( ) # . .. ... : ; ? \\ [ ] { } _ ~ "
                       "== != >= <= += -= /= %= *= ~=";
    // std::vector<Token::Type> correct {Token::Add,Token::Sub,Token::Mul};
    auto file = FSFile("", code);
    Lexer l = Lexer(file);
    auto t = l.next();
    EXPECT_EQ(t.type, Token::Add);
    t = l.next();
    EXPECT_EQ(t.type, Token::Sub);
    t = l.next();
    EXPECT_EQ(t.type, Token::Mul);
    t = l.next();
    EXPECT_EQ(t.type, Token::Div);
    t = l.next();
    EXPECT_EQ(t.type, Token::Mod);
    t = l.next();
    EXPECT_EQ(t.type, Token::Xor);
    t = l.next();
    EXPECT_EQ(t.type, Token::Not);
    t = l.next();
    EXPECT_EQ(t.type, Token::Eq);
    t = l.next();
    EXPECT_EQ(t.type, Token::And);
    t = l.next();
    EXPECT_EQ(t.type, Token::Or);
    t = l.next();
    EXPECT_EQ(t.type, Token::Lp);
    t = l.next();
    EXPECT_EQ(t.type, Token::Rp);
    t = l.next();
    EXPECT_EQ(t.type, Token::Hashtag);
    t = l.next();
    EXPECT_EQ(t.type, Token::Dot);
    t = l.next();
    EXPECT_EQ(t.type, Token::DotDot);
    t = l.next();
    EXPECT_EQ(t.type, Token::DotDotDot);
    t = l.next();
    EXPECT_EQ(t.type, Token::DoubleDot);
    t = l.next();
    EXPECT_EQ(t.type, Token::SemiColon);
    t = l.next();
    EXPECT_EQ(t.type, Token::Questionmark);
    t = l.next();
    EXPECT_EQ(t.type, Token::Backslash);
    t = l.next();
    EXPECT_EQ(t.type, Token::Lb);
    t = l.next();
    EXPECT_EQ(t.type, Token::Rb);
    t = l.next();
    EXPECT_EQ(t.type, Token::Lc);
    t = l.next();
    EXPECT_EQ(t.type, Token::Rc);
    t = l.next();
    EXPECT_EQ(t.type, Token::Underscore);
    t = l.next();
    EXPECT_EQ(t.type, Token::Neg);
    t = l.next();
    EXPECT_EQ(t.type, Token::EqEq);
    t = l.next();
    EXPECT_EQ(t.type, Token::NotEq);
    t = l.next();
    EXPECT_EQ(t.type, Token::GtEq);
    t = l.next();
    EXPECT_EQ(t.type, Token::LtEq);
    t = l.next();
    EXPECT_EQ(t.type, Token::AddEq);
    t = l.next();
    EXPECT_EQ(t.type, Token::SubEq);
    t = l.next();
    EXPECT_EQ(t.type, Token::DivEq);
    t = l.next();
    EXPECT_EQ(t.type, Token::ModEq);
    t = l.next();
    EXPECT_EQ(t.type, Token::MulEq);
    t = l.next();
    EXPECT_EQ(t.type, Token::NegEq);
}

TEST(Lex, OneInOther) {
    std::string code = "addi32i32";
    auto file = FSFile("", code);
    Lexer l = Lexer(file);
    auto t = l.next();
    EXPECT_EQ(t.type, Token::Id);
    t.getName();
}
