#pragma once
#include "compatibility.h"
#include "context.h"
#include "source.h"
#include "type.h"
#include <string>
#include <variant>

struct FSFile;

ptr hash(const std::string &str);
class SourceLocation {
public:
  unsigned int pos, indent;
  std::string::const_iterator it, end;
  SourceLocation(FSFile &file);
  INLINE char peek(const int n = 0);
  INLINE char pop();
  INLINE bool can_iter();
  SourceLocation &operator=(const SourceLocation &other);
  SourceLocation get_sourcelocation();
};

enum Kw_e : unsigned char {
  Unk,
  Fn,
  For,
  I8,
  I16,
  I32,
  I64,
  String,
  Drop, /* _ */
  If,
  Import,
  Export,
  Extern,
  Module,
  True,
  False,
  Bool
};

bool is_op(u8 ch);
bool is_ws(u8 ch);
Kw_e is_kw(ptr h);

struct Lit {
    IntegralType ty;
    union {
        unsigned char u8;
        unsigned short u16;
        unsigned int u32;
        unsigned long u64;
        char i8;
        short i16;
        int i32;
        long i64;
        bool b;
        float f32;
        double f64;
        std::string_view string="";
    } as;
    Lit(unsigned char u8);
    Lit(unsigned short u16);
    Lit(unsigned int u32);
    Lit(unsigned long u64);
    Lit(char i8);
    Lit(short i16);
    Lit(int i32);
    Lit(long i64);
    Lit(bool b);
    Lit(float f32);
    Lit(double f64);
    Lit(std::string_view string);

};

struct Token {
  enum Type : unsigned char {
    Eof,
    Id,
    Kw,

    N,
    Gi,
    Li,

    Lit,

    Not = 64,
    Hashtag,
    Mod,
    Lp,
    Rp,
    Mul,
    Add,
    Sub,
    Comma,
    Dot,
    DotDot,
    DotDotDot,
    Div,
    DoubleDot,
    SemiColon,
    Gt, //<
    Lt, // >
    Eq,
    Questionmark,
    Backslash,
    Lb, //[
    Rb, // ]
    Underscore,
    Xor, //^
    Lc,  //{
    Rc,  //}
    Or,
    Neg, //~
    Null,
    Space,
    Tab,
    And,

    EqEq,
    NotEq,
    GtEq,
    LtEq,
    AddEq,
    SubEq,
    DivEq,
    ModEq,
    MulEq,
    NegEq,


  } type;
  SourceLocation sl;

  Token(Type type, const SourceLocation &sl);
  Token(u8 c, const SourceLocation &sl);
  Token(::Lit val, const SourceLocation &sl);
  Token(const std::string &str, const SourceLocation &sl);
  Token(Kw_e kw, const SourceLocation &sl);
  Token &operator=(const Token &other);
  ::Lit getValue() const;
  std::string getName() const;
  Kw_e getKw() const;
  std::variant<Kw_e, ::Lit, std::string> data;

private:
};


class Lexer : public SourceLocation {
public:
  Lexer(FSFile &file);
  Token next();
  void lex();
  void test();
  std::vector<Token> tokens;

private:
  FSFile &file;
  char lex_escape(const char esc);
  Lit nolit(const SourceLocation &s, bool f, int base);
  Lit stringlit(std::string s);
  Token lex_id_or_kw(SourceLocation& err_loc);
  Token lex_number(SourceLocation& err_loc);
  Token lex_string(SourceLocation& err_loc);
  Token lex_newline(SourceLocation& err_loc);
  Token lex_dots(SourceLocation& err_loc);
  Token lex_eq(SourceLocation& err_loc);
  Token lex_mul(SourceLocation& err_loc);
  Token lex_div(SourceLocation& err_loc);
  Token lex_not(SourceLocation& err_loc);
  Token lex_gt(SourceLocation& err_loc);
  Token lex_lt(SourceLocation& err_loc);
  Token lex_add(SourceLocation& err_loc);
  Token lex_sub(SourceLocation& err_loc);
  Token lex_mod(SourceLocation& err_loc);
  Token lex_neg(SourceLocation& err_loc);
};
