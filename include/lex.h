#pragma once
#include "compatibility.h"
#include "context.h"
#include "source.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Value.h"
#include <string>
#include <variant>
#include "type.h"

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
};

bool is_op(u8 ch);
bool is_ws(u8 ch);
Kw_e is_kw(ptr h);
bool is_eol(u8 ch);

struct Lit {
  Type* ty;
  llvm::Constant* val;
  Lit(Type* ty,llvm::Constant* val): ty(ty),val(val){}
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
template <typename T> static SourceLocation sl_cast(T *l) {
  return *reinterpret_cast<SourceLocation *>(l);
}

class Lexer : public SourceLocation {
public:
  Lexer(FSFile &file, FusionCtx &ctx);
  Token next();
  void lex();
  void test();
  std::vector<Token> tokens;

private:
  FusionCtx &ctx;
  FSFile &file;
  char lex_escape(const char esc);
  Lit nolit(const SourceLocation &s, bool f, int base);
  Lit stringlit(std::string s);
};
