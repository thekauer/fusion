#include "context.h"
#include "lex.h"
#include "source.h"
#include "gtest/gtest.h"

TEST(Lex, Keywords) {
  // test _
  std::string code = "fn for i8 i16 i32 i64 string if import export extern mod "
                     "true false bool";
  std::vector<Kw_e> tkns{ Kw_e::Fn,Kw_e::For,Kw_e::I8,Kw_e::I16,Kw_e::I32,Kw_e::I64,Kw_e::String,Kw_e::If,Kw_e::Import,Kw_e::Export,Kw_e::Extern,Kw_e::Module,Kw_e::True,Kw_e::False,Kw_e::Bool };
  auto file = FSFile("", code);
  Lexer l = Lexer(file);
  for (auto expected : tkns) {
      auto t = l.next();
      EXPECT_EQ(t.type, Token::Kw);
      EXPECT_EQ(t.getKw(), expected);
  }
}

TEST(Lex, Operator) {
  std::string code = "+ - * / % ^ ! = & | ( ) # . .. ... : ; ? \\ [ ] { } _ ~ "
                     "== != >= <= += -= /= %= *= ~=";
  std::vector<Token::Type> correct {Token::Add,Token::Sub,Token::Mul,Token::Div,Token::Mod,Token::Xor,Token::Not,Token::Eq,
  Token::And,Token::Or,Token::Lp,Token::Rp,Token::Hashtag,Token::Dot,Token::DotDot,Token::DotDotDot,Token::DoubleDot,Token::SemiColon,
  Token::Questionmark,Token::Backslash,Token::Lb,Token::Rb,Token::Lc,Token::Rc,Token::Underscore,Token::Neg,Token::EqEq,Token::NotEq,
  Token::GtEq,Token::LtEq,Token::AddEq,Token::SubEq,Token::DivEq,Token::ModEq,Token::MulEq,Token::NegEq};
  auto file = FSFile("", code);
  Lexer l = Lexer(file);
  for (auto expected : correct) {
      auto t = l.next();
      EXPECT_EQ(t.type, expected);
  }
}

TEST(Lex, OneInOther) {
  std::string code = "addi32i32";
  auto file = FSFile("", code);
  Lexer l = Lexer(file);
  auto t = l.next();
  EXPECT_EQ(t.type, Token::Id);
  t.getName();
}

TEST(Lex, Indent) {
    std::string code = "\n\n  \n \n\n ";
    auto file = FSFile("", code);
    Lexer l = Lexer(file);
    std::vector<Token::Type> correct  { Token::N,Token::Gi,Token::Li,Token::Li };
    for (auto expected : correct) {
        EXPECT_EQ(expected, l.next().type);
    }
}

TEST(Lex, EmptyLines) {
    std::string code = "fn\n 1\n\nfn\n 1";
    auto file = FSFile("", code);
    Lexer l = Lexer(file);
    std::vector<Token::Type> correct{ Token::Kw,Token::Gi,Token::Lit,Token::Li,Token::N,Token::Kw,Token::Gi,Token::Lit};
    for (auto expected : correct) {
        EXPECT_EQ(expected, l.next().type);
    }
}