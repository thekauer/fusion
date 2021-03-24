#include "context.h"
#include "lex.h"
#include "parser.h"
#include "source.h"
#include "gtest/gtest.h"

TEST(parser, only_primary) {
  FusionCtx ctx;
  std::string code = "2";
  auto file = FSFile("test", code);
  auto lexer = Lexer(file);
  lexer.lex();
  auto tokens = lexer.tokens;
  Token::Type tks[]{Token::Lit};
  EXPECT_EQ(tokens.size(), 1);
  for (int i = 0; i < tokens.size(); i++) {
    EXPECT_EQ(tokens[i].type, tks[i]) << "i: " << i;
  }
  auto p = Parser(tokens, ctx, file);
  auto expr = p.parse_expr();
  ASSERT_NE(expr, nullptr) << "expr is null";
  ASSERT_EQ(expr->ast_type, AstType::ValExpr);
}

TEST(parser, primary_and_op) {
  FusionCtx ctx;
  std::string code = "2+";
  auto file = FSFile("test", code);
  auto lexer = Lexer(file);
  lexer.lex();
  auto tokens = lexer.tokens;
  Token::Type tks[]{Token::Lit, Token::Add};
  EXPECT_EQ(tokens.size(), 2);
  for (int i = 0; i < tokens.size(); i++) {
    EXPECT_EQ(tokens[i].type, tks[i]) << "i: " << i;
  }
  auto p = Parser(tokens, ctx, file);
  try {
    auto expr = p.parse_expr();
    ASSERT_EQ(expr, nullptr) << "expr is null";
  } catch (const std::exception &e) {
    ASSERT_EQ(1, 0) << "crashed";
  }
}
TEST(parser, outofbounnds) {
  FusionCtx ctx;
  std::string code = "2 + 3";
  auto file = FSFile("test", code);
  auto lexer = Lexer(file);
  lexer.lex();
  auto tokens = lexer.tokens;
  Token::Type tks[]{Token::Lit, Token::Add, Token::Lit};
  EXPECT_EQ(tokens.size(), 3);
  for (int i = 0; i < tokens.size(); i++) {
    EXPECT_EQ(tokens[i].type, tks[i]) << "i: " << i;
  }
  /*
          +
        /   \
       2     3
  */

  auto p = Parser(tokens, ctx, file);
  try {
    auto expr = p.parse_expr();
    ASSERT_NE(expr, nullptr) << "expr is null";
    ASSERT_EQ(expr->ast_type, AstType::BinExpr);
    auto bin = reinterpret_cast<BinExpr *>(expr.get());
    EXPECT_EQ(bin->lhs->ast_type, AstType::ValExpr);
    EXPECT_EQ(bin->rhs->ast_type, AstType::ValExpr);
  } catch (const std::exception &e) {
    ASSERT_EQ(1, 0) << "crashed";
  }
}
TEST(parser, addition) {
  FusionCtx ctx;
  std::string code = "2 + 3 * 2\n";
  auto file = FSFile("test", code);
  auto lexer = Lexer(file);
  lexer.lex();
  auto tokens = lexer.tokens;
  EXPECT_EQ(tokens.size(), 6);
  Token::Type tks[]{Token::Lit, Token::Add, Token::Lit,
                    Token::Mul, Token::Lit, Token::N};
  for (int i = 0; i < tokens.size(); i++) {
    EXPECT_EQ(tokens[i].type, tks[i]) << "i: " << i;
  }
  /*
          *
        /   \
       +     2
      / \
     2   3
  */
  auto p = Parser(tokens, ctx, file);
  auto expr = p.parse_expr();
  ASSERT_NE(expr, nullptr) << "expr is null";
  ASSERT_EQ(expr->ast_type, AstType::BinExpr);
  auto bin = reinterpret_cast<BinExpr *>(expr.get());

  EXPECT_EQ(bin->lhs->ast_type, AstType::ValExpr);

  ASSERT_EQ(bin->rhs->ast_type, AstType::BinExpr);
  auto r = reinterpret_cast<BinExpr *>(bin->rhs.get());
  EXPECT_EQ(r->lhs->ast_type, AstType::ValExpr);
  EXPECT_EQ(r->rhs->ast_type, AstType::ValExpr);
}
TEST(parser, binexpr) {
  FusionCtx ctx;
  std::string code = "5 = 3 + 2";
  auto file = FSFile("test", code);
  auto lexer = Lexer(file);
  lexer.lex();
  auto tokens = lexer.tokens;
  EXPECT_EQ(tokens.size(), 5);
  Token::Type tks[]{Token::Lit, Token::Eq, Token::Lit, Token::Add, Token::Lit};
  for (int i = 0; i < tokens.size(); i++) {
    EXPECT_EQ(tokens[i].type, tks[i]) << "i: " << i;
  }
  /*
          =
        /   \
       5     +
           /   \
          3     2
  */
  auto p = Parser(tokens, ctx, file);
  auto expr = p.parse_expr();
  ASSERT_NE(expr, nullptr) << "expr is null";
  ASSERT_EQ(expr->ast_type, AstType::BinExpr);
  auto bin = reinterpret_cast<BinExpr *>(expr.get());
  EXPECT_EQ(bin->lhs->ast_type, AstType::ValExpr) << "left should be varexpr (a=)";
  ASSERT_EQ(bin->rhs->ast_type, AstType::BinExpr) << "right side should be 3+2";
  /*
  auto r = reinterpret_cast<BinExpr*>(bin->rhs.get());
  EXPECT_EQ(r->op,Token::Add) << "Expected a +";
  EXPECT_EQ(r->lhs->type,AstType::ValExpr) << "3 should be a val";
  EXPECT_EQ(r->rhs->type,AstType::ValExpr) << "2 should be a type";
  */
}
TEST(parser, IfStmt) {
  FusionCtx ctx;
  std::string code = "if true\n 1+1\nelse\n 1+1\n";
  auto file = FSFile("test", code);
  auto lexer = Lexer(file);
  lexer.lex();
  auto p = Parser(lexer.tokens, ctx, file);
  auto expr = p.parse_expr();
  auto ifstmt = reinterpret_cast<IfStmt *>(expr.get());
  ASSERT_NE(ifstmt, nullptr);

  ASSERT_NE(ifstmt->condition, nullptr);
  EXPECT_EQ(ifstmt->condition->cast<ValExpr>()->val.as.i32, 1);

  ASSERT_NE(ifstmt->body, nullptr);
  EXPECT_EQ(ifstmt->body->body.size(), 1);
  EXPECT_EQ(ifstmt->body->body[0]->ast_type, AstType::BinExpr);
  auto body = ifstmt->body->body[0]->cast<BinExpr>();
  EXPECT_EQ(body->lhs->ast_type, AstType::ValExpr);
  EXPECT_EQ(body->lhs->cast<ValExpr>()->val.as.i32, 1);
  EXPECT_EQ(body->rhs->ast_type, AstType::ValExpr);
  EXPECT_EQ(body->rhs->cast<ValExpr>()->val.as.i32, 1);

  ASSERT_NE(ifstmt->else_body, nullptr);
  EXPECT_EQ(ifstmt->else_body->body.size(), 1);
  auto else_body = ifstmt->else_body->body[0]->cast<BinExpr>();
  EXPECT_EQ(else_body->lhs->ast_type, AstType::ValExpr);
  EXPECT_EQ(else_body->lhs->cast<ValExpr>()->val.as.i32, 1);
  EXPECT_EQ(else_body->rhs->ast_type, AstType::ValExpr);
  EXPECT_EQ(else_body->rhs->cast<ValExpr>()->val.as.i32, 1);
}
TEST(parser, ResolveType) {
  FusionCtx ctx;
  /*
  class A
   a : i32
   b : i32
  fn main()
   a : A
  */
  std::string code = "class A\n a : i32\n b : i32\n\nfn main()\n a : A\n";
  auto file = FSFile("test", code);
  auto lexer = Lexer(file);
  lexer.lex();
  auto p = Parser(lexer.tokens, ctx, file);
  
  auto body = p.parse();
  ASSERT_EQ(body->body.size(), 2); // class ,fndecl
  auto class_stmt = body->body[0].get();
  ASSERT_EQ(class_stmt->ast_type, AstType::ClassStmt);
  auto fndecl = body->body[1].get();
  ASSERT_EQ(fndecl->ast_type, AstType::FnDecl);

  auto class_body = class_stmt->cast<ClassStmt>()->body.get();
  ASSERT_EQ(class_body->body.size(), 2); // 2x vardecl

  ASSERT_EQ(class_body->body[0]->ast_type, AstType::VarDeclExpr);
  auto a_ty = class_body->body[0]->cast<VarDeclExpr>()->ty;
  ASSERT_EQ(a_ty.get_type_ptr()->get_typekind(), Type::Integral);
  EXPECT_EQ(reinterpret_cast<const IntegralType *>(a_ty.get_type_ptr())->ty,
            IntegralType::I32);

  ASSERT_EQ(class_body->body[1]->ast_type, AstType::VarDeclExpr);
  auto b_ty = class_body->body[1]->cast<VarDeclExpr>()->ty;
  ASSERT_EQ(b_ty.get_type_ptr()->get_typekind(), Type::Integral);
  EXPECT_EQ(reinterpret_cast<const IntegralType *>(b_ty.get_type_ptr())->ty,
            IntegralType::I32);

  auto fn_body = fndecl->cast<FnDecl>()->body.get();
  ASSERT_EQ(fn_body->body.size(), 1); // vardecl
  ASSERT_EQ(fn_body->body[0]->ast_type, AstType::VarDeclExpr);

}

TEST(parser, VarDeclExpr) {
  FusionCtx ctx;
  std::string code = "fn main()\n a : i32\n b : i32 = 1\n";
  auto file = FSFile("test", code);
  auto lexer = Lexer(file);
  lexer.lex();
  auto tokens = lexer.tokens;
  Token::Type tks[]{Token::Kw,Token::Id,Token::Lp,Token::Rp,Token::Gi, // fn main()
                    Token::Id,Token::DoubleDot, Token::Kw,Token::N, // a : i32
                    Token::Id,Token::DoubleDot,Token::Kw,Token::Eq,Token::Lit,Token::Li // b : i32 = 1
                    };
  ASSERT_EQ(tokens.size(), sizeof(tks)/sizeof(tks[0]) );
  for (int i = 0; i < tokens.size(); i++) {
    ASSERT_EQ(tokens[i].type, tks[i]) << "i: " << i;
  }
  auto p = Parser(tokens, ctx, file);
  auto tle = p.parse();
  ASSERT_NE(tle, nullptr) << "top level expr is null";
  ASSERT_EQ(tle->ast_type, AstType::Body);
  ASSERT_EQ(tle->body.size(), 1);
  auto main = tle->body[0].get();
  ASSERT_EQ(main->ast_type, AstType::FnDecl);
  auto body = main->cast<FnDecl>()->body.get();
  ASSERT_EQ(body->body.size(), 2);
  auto first = body->body[0].get();
  ASSERT_EQ(first->ast_type, AstType::VarDeclExpr);
  auto second = body->body[1].get();
  ASSERT_EQ(second->ast_type, AstType::BinExpr);

  auto first_vardecl = first->cast<VarDeclExpr>();
  auto second_binexpr = second->cast<BinExpr>();

  ASSERT_EQ(first_vardecl->name, "a");
  ASSERT_EQ(first_vardecl->ty.get_type_ptr(), &Type::get_i32());

  ASSERT_EQ(second_binexpr->lhs->ast_type, AstType::VarDeclExpr);
  ASSERT_EQ(second_binexpr->rhs->ast_type, AstType::ValExpr);
  auto second_lhs = second_binexpr->lhs->cast<VarDeclExpr>();
  auto second_rhs = second_binexpr->rhs->cast<ValExpr>();

  ASSERT_EQ(second_lhs->name, "b");
  ASSERT_EQ(second_lhs->ty.get_type_ptr(), &Type::get_i32());
  ASSERT_EQ(second_rhs->val.ty.get_type_ptr(), &Type::get_i32());
  ASSERT_EQ(second_rhs->val.as.i32, 1);
}