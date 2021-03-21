#include "lex.h"
#include <map>


enum Eq : unsigned char {
  Not = Token::Not,
  Hashtag = Token::Hashtag,
  Mod = Token::Mod,
  Lp = Token::Lp,
  Rp = Token::Rp,
  Mul = Token::Mul,
  Add = Token::Add,
  Sub = Token::Sub,
  Comma = Token::Comma,
  Dot = Token::Dot,
  Div = Token::Div,
  DoubleDot = Token::DoubleDot,
  SemiColon = Token::SemiColon,
  Gt = Token::Gt, //<
  Lt = Token::Lt, // >
  Eq = Token::Eq,
  Questionmark = Token::Questionmark,
  Backslash = Token::Backslash,
  Lb = Token::Lb, //[
  Rb = Token::Rb, // ]
  Underscore = Token::Underscore,
  Triangle = Token::Xor, //^
  Lc = Token::Lc,        //{
  Rc = Token::Rc,        //}
  Or = Token::Or,
  Neg = Token::Neg, //~
  Null = Token::Null,
  Space,
  Tab,
  And = Token::And,
  // parsing required
  Quote,
  Apostrophe,
  Cr = Token::N, // carrige return
  N = Token::N,  // ascii 10 == 0xA
  Letter = 100,
  Number = 128,
  NotAToken = 255,
};
static const unsigned eq[128] = {
    Null,      NotAToken, NotAToken, NotAToken,    NotAToken, NotAToken,
    NotAToken, NotAToken, NotAToken, NotAToken,    N,         NotAToken,
    NotAToken, Cr,        NotAToken, NotAToken,    NotAToken, NotAToken,
    NotAToken, NotAToken, NotAToken, NotAToken,    NotAToken, NotAToken,
    NotAToken, NotAToken, NotAToken, NotAToken,    NotAToken, NotAToken,
    NotAToken, NotAToken, Space,     Not,          Quote,     Hashtag,
    NotAToken, Mod,       And,       Apostrophe,   Lp,        Rp,
    Mul,       Add,       Comma,     Sub,          Dot,       Div,
    Number,    Number,    Number,    Number,       Number,    Number,
    Number,    Number,    Number,    Number,       DoubleDot, SemiColon,
    Lt,        Eq,        Gt,        Questionmark, NotAToken, Letter,
    Letter,    Letter,    Letter,    Letter,       Letter,    Letter,
    Letter,    Letter,    Letter,    Letter,       Letter,    Letter,
    Letter,    Letter,    Letter,    Letter,       Letter,    Letter,
    Letter,    Letter,    Letter,    Letter,       Letter,    Letter,
    Letter,    Lb,        Backslash, Rb,           Triangle,  Underscore,
    NotAToken, Letter,    Letter,    Letter,       Letter,    Letter,
    Letter,    Letter,    Letter,    Letter,       Letter,    Letter,
    Letter,    Letter,    Letter,    Letter,       Letter,    Letter,
    Letter,    Letter,    Letter,    Letter,       Letter,    Letter,
    Letter,    Letter,    Letter,    Lc,           Or,        Rc,
    Neg};

Lit::Lit(unsigned char u8) : ty(QualType(Type::get_u8())) { as.u8 = u8; }
Lit::Lit(unsigned short u16) : ty(QualType(Type::get_u16())) { as.u16 = u16; }
Lit::Lit(unsigned int u32) : ty(QualType(Type::get_u16())) { as.u32 = u32; }
Lit::Lit(unsigned long u64) : ty(QualType(Type::get_u64())) { as.u64 = u64; };
Lit::Lit(char i8) : ty(QualType(Type::get_i8())) { as.i8 = i8; };
Lit::Lit(short i16) : ty(QualType(Type::get_i16())) { as.i16 = i16; };
Lit::Lit(int i32) : ty(QualType(Type::get_i32())) { as.i32 = i32; };
Lit::Lit(long i64) : ty(QualType(Type::get_i64())) { as.i64 = i64; };
Lit::Lit(bool b) : ty(QualType(Type::get_bool())) { as.b = b; };
Lit::Lit(float f32) : ty(QualType(Type::get_f32())) { as.f32 = f32; };
Lit::Lit(double f64) : ty(QualType(Type::get_f64())) { as.f64 = f64; };
Lit::Lit(std::string_view string) : ty(QualType(Type::get_string())) {
  as.string = string;
};

Lit &Lit::operator=(const Lit &other) {
  ty = other.ty;
  as = other.as;
  return *this;
}

Token::Token(Type type, const SourceLocation &sl) : type(type), sl(sl){};
Token::Token(unsigned char c, const SourceLocation &sl)
    : type(static_cast<Type>(c)), sl(sl) {}
Token::Token(::Lit val, const SourceLocation &sl)
    : type(Lit), sl(sl), data(val) {}
Token::Token(const std::string &str, const SourceLocation &sl)
    : type(Id), sl(sl), data(str) {}
Token::Token(Kw_e kw, const SourceLocation &sl) : type(Kw), sl(sl), data(kw) {}

Lit Token::getValue() const { return std::get<::Lit>(data); }
std::string Token::getName() const { return std::get<std::string>(data); }
Kw_e Token::getKw() const { return std::get<Kw_e>(data); }

bool is_op(unsigned char ch) { return ch >= Not && ch <= And; }
bool is_ws(unsigned char ch) { return ch == Tab || ch == Space; }

static const std::map<std::string, Kw_e> kws{
    {"fn", Fn},         {"for", For},
    {"i8", I8},         {"i16", I16},
    {"i32", I32},       {"i64", I64},
    {"string", String}, {"_", Drop},
    {"if", If},         {"else", Else},
    {"import", Import}, {"return", Return},
    {"extern", Extern}, {"export", Export},
    {"mod", Module},    {"true", True},
    {"false", False},   {"bool", Bool},
    {"class", Class},
};
Kw_e is_kw(const std::string& h) {
  auto k = kws.find(h);
  if (k != kws.end()) {
    return k->second;
  }
  return Unk;
}

Lit Lexer::nolit(const SourceLocation &s, bool f, int base) {
  std::string str(s.it, it);
  if (f) {
    float D = 0.0;
    D = std::stod(str);
    return Lit(D);
  } else {
    int I = 0;
    I = std::stoi(str);
    return Lit(I);
  }
}

Lit Lexer::stringlit(std::string s) { return Lit(s); }


void Lexer::lex() {
  if (!can_iter()) {
    serror(Error_e::Unk, "Empty file");
  }
  while (can_iter()) {
    tokens.push_back(next());
  }
}
Token Lexer::lex_id_or_kw(SourceLocation &err_loc) {
  pop();
  while ((eq[peek()] == Letter || eq[peek()] == Number ||
          eq[peek()] == Underscore) &&
         can_iter()) {
    pop();
  }
  auto e = this->get_sourcelocation();
  auto s = std::string(err_loc.it, e.it);
  auto h = s;
  auto k = is_kw(h);
  if (k != Unk) {
    return Token(k, e);
  }
  return Token(s, e);
}
Token Lexer::lex_number(SourceLocation &err_loc) {
  // Handle base
  int base = 10;
  while (eq[peek()] == Number) {
    pop();
  }
  bool is_float = false;
  if (eq[peek()] == Dot && eq[peek(1)] == Number) {
    pop();
    is_float = true;
  }
  while (eq[peek()] == Number) {
    pop();
  }

  return Token(nolit(err_loc, is_float, base), err_loc);
}

Token Lexer::lex_string(SourceLocation &err_loc) {
  pop(); // pop the quote
  std::string buff;
  while (eq[peek()] != Quote) {
    if (eq[peek()] == Backslash) {
      pop(); // pop backlash
      buff += lex_escape(pop());
      continue;
    }
    buff += pop();
  }
  pop(); // pop the ending quote

  return Token(stringlit(buff), err_loc);
}

Token Lexer::lex_newline(SourceLocation &err_loc) {
  pop(); // pop the N
  auto curr_indent = 0;
  while (eq[peek()] == Space) {
    pop();
    curr_indent++;
  }
  while (eq[peek()] == Tab) {
    pop();
    curr_indent++;
  }
  while (eq[peek()] == Token::N) {
    return lex_newline(err_loc);
  }

  if (indent < curr_indent) {
    indent = curr_indent;
    return Token(Token::Gi, this->get_sourcelocation());
  }
  if (indent > curr_indent) {
    li = indent - curr_indent -1;
    indent = curr_indent;
    return Token(Token::Li, this->get_sourcelocation());
  }
  return Token(Token::N, err_loc);
}

Token Lexer::lex_dots(SourceLocation &err_loc) {
  pop();
  if (eq[peek()] == Dot) {
    pop();
    if (eq[peek()] == Dot) {
      pop();
      return Token(Token::DotDotDot, this->get_sourcelocation());
    }
    return Token(Token::DotDot, this->get_sourcelocation());
  }
  return Token(Token::Dot, this->get_sourcelocation());
}

Token Lexer::lex_eq(SourceLocation &err_loc) {
  pop();
  if (eq[peek(1)] == Eq) {
    pop();
    return Token(Token::EqEq, err_loc);
  } else {
    return Token(Token::Eq, err_loc);
  }
}

Token Lexer::lex_mul(SourceLocation &err_loc) {
  pop();
  if (eq[peek(1)] == Eq) {
    pop();
    return Token(Token::MulEq, err_loc);
  } else {
    return Token(Token::Mul, err_loc);
  }
}

Token Lexer::lex_div(SourceLocation &err_loc) {
  // implement comments
  pop();
  if (eq[peek(1)] == Eq) {
    pop();
    return Token(Token::DivEq, err_loc);
  }
  return Token(Token::Div, err_loc);
}

Token Lexer::lex_not(SourceLocation &err_loc) {
  pop();
  if (eq[peek(1)] == Eq) {
    pop();
    return Token(Token::NotEq, err_loc);
  } else {
    return Token(Token::Not, err_loc);
  }
}

Token Lexer::lex_gt(SourceLocation &err_loc) {
  pop();
  auto n = eq[peek(1)];
  if (n == Eq) {
    pop();
    return Token(Token::GtEq, err_loc);
  }
  return Token(Token::Gt, err_loc);
}

Token Lexer::lex_lt(SourceLocation &err_loc) {
  pop();
  auto n = eq[peek(1)];
  if (n == Eq) {
    pop();
    return Token(Token::LtEq, err_loc);
  }
  return Token(Token::Lt, err_loc);
}

Token Lexer::lex_add(SourceLocation &err_loc) {
  pop();
  if (eq[peek(1)] == Eq) {
    pop();
    return Token(Token::AddEq, err_loc);
  }
  return Token(Token::Add, err_loc);
}

Token Lexer::lex_sub(SourceLocation &err_loc) {
  pop();
  if (eq[peek(1)] == Eq) {
    pop();
    return Token(Token::SubEq, err_loc);
  }
  return Token(Token::Sub, err_loc);
}

Token Lexer::lex_mod(SourceLocation &err_loc) {
  pop();
  if (eq[peek(1)] == Eq) {
    pop();
    return Token(Token::ModEq, err_loc);
  }
  return Token(Token::Mod, err_loc);
}

Token Lexer::lex_neg(SourceLocation &err_loc) {
  pop();
  if (eq[peek(1)] == Eq) {
    pop();
    return Token(Token::NegEq, err_loc);
  }
  return Token(Token::Neg, err_loc);
}

Lexer::Lexer(FSFile &file) : SourceLocation(file), file(file){};
Token Lexer::next() {
  while (eq[peek()] == Space) {
    pop();
  }
  SourceLocation err_loc = this->get_sourcelocation();

  unsigned char ch = eq[peek()];
  switch (ch) {
  case Letter:
    return lex_id_or_kw(err_loc);
  case Number:
    return lex_number(err_loc);
  case Quote:
    return lex_string(err_loc);
  case N:
    return lex_newline(err_loc);
  case Dot:
    return lex_dots(err_loc);
  case Eq:
    return lex_eq(err_loc);
  case Mul:
    return lex_mul(err_loc);
  case Div:
    return lex_div(err_loc);
  case Not:
    return lex_not(err_loc);
  case Gt:
    return lex_gt(err_loc);
  case Lt:
    return lex_lt(err_loc);
  case Add:
    return lex_add(err_loc);
  case Sub:
    return lex_sub(err_loc);
  case Mod:
    return lex_mod(err_loc);
  case Neg:
    return lex_neg(err_loc);

  default:
    break;
  }

  if (is_op(ch)) {
    pop(); // pop the operator, it is stored in ch
    // Add commnet lexer here
    return Token(ch, this->get_sourcelocation());
  }
  LLVM_BUILTIN_UNREACHABLE;
}
char Lexer::lex_escape(const char esc) {
  auto err_loc = *reinterpret_cast<SourceLocation *>(this);
  switch (esc) {
  case 'r':
    return '\r';
    break;
  case 'n':
    return '\n';
    break;
  case '\\':
    return '\\';
    break;
  case 'b':
    return '\b';
    break;
  case '\'':
    return '\'';
    break;
  case '\"':
    return '\"';
    break;
  case 't':
    return '\t';
    break;
  case 'v':
    return '\v';
    break;
  default:
    Error::UnkEscapeChar(file, err_loc, esc);
    break;
  }
  return '\0';
}

INLINE char SourceLocation::peek(const int n) { return *it; }

INLINE char SourceLocation::pop() {
  char r = *it;
  pos++;
  prefetch(&it);
  ++it;
  return r;
}

INLINE bool SourceLocation::can_iter() { return it != end; }

void Lexer::test() {
  while (can_iter()) {
    std::cout << pop();
  }
}

SourceLocation::SourceLocation(FSFile &file)
    : pos(0), indent(0),li(0), it(file.code.begin()), end(file.code.end()) {}

SourceLocation &SourceLocation::operator=(const SourceLocation &other) {
  pos = other.pos;
  indent = other.indent;
  it = other.it;
  end = other.end;
  li = other.li;
  return *this;
}

SourceLocation SourceLocation::get_sourcelocation() { return *this; }
