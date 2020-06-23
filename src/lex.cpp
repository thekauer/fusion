#include "lex.h"
#include "llvm/IR/Constants.h"
#include <map>
ptr hash(const std::string &str) {

  u32 h = 123456;
  u32 len = str.size();
  const u8 *key = (const u8 *)(&str[0]);
  if (len > 3) {
    const u32 *key_x4 = (const uint32_t *)(&str[0]);
    size_t i = len >> 2;
    do {
      u32 k = *key_x4++;
      k *= 0xcc9e2d51;
      k = (k << 15) | (k >> 17);
      k *= 0x1b873593;
      h ^= k;
      h = (h << 13) | (h >> 19);
      h = h * 5 + 0xe6546b64;
    } while (--i);
    key = (const u8 *)key_x4;
  }
  if (len & 3) {
    size_t i = len & 3;
    u32 k = 0;
    key = &key[i - 1];
    do {
      k <<= 8;
      k |= *key--;
    } while (--i);
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    h ^= k;
  }
  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

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
  NotAToken=255,
};
static const unsigned eq[128] = {Null,     NotAToken,     NotAToken,        NotAToken,
                                NotAToken,        NotAToken,     NotAToken,        NotAToken,
                                NotAToken,        NotAToken,      N,        NotAToken,
                                NotAToken,         Cr,    NotAToken,        NotAToken,
                                NotAToken,        NotAToken,     NotAToken,        NotAToken,
                                NotAToken,        NotAToken,     NotAToken,        NotAToken,
                                NotAToken,        NotAToken,     NotAToken,        NotAToken,
                                NotAToken,        NotAToken,     NotAToken,        NotAToken,
                                 Space,     Not,    Quote,     Hashtag,
                                NotAToken,         Mod,    And,       Apostrophe,
                                 Lp,        Rp,     Mul,       Add,
                                 Comma,     Sub,    Dot,       Div,
                                 Number,    Number, Number,    Number,
                                 Number,    Number, Number,    Number,
                                 Number,    Number, DoubleDot, SemiColon,
                                 Lt,        Eq,     Gt,        Questionmark,
                                NotAToken,         Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Lb,
                                 Backslash, Rb,     Triangle,  Underscore,
                                NotAToken,         Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Lc,
                                 Or,        Rc,     Neg};


Lit::Lit(unsigned char u8) : as{ u8 },IntegralType("u8",IntegralType::U8,1) {}
Lit(unsigned short u16) : as{u16},IntegralType("U16")
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

Token::Token(Type type, const SourceLocation &sl) : type(type), sl(sl){};
Token::Token(u8 c, const SourceLocation &sl)
    : type(static_cast<Type>(c)), sl(sl) {}
Token::Token(::Lit val, const SourceLocation &sl)
    : type(Lit), sl(sl), data(val) {}
Token::Token(const std::string &str, const SourceLocation &sl)
    : type(Id), sl(sl), data(str) {}
Token::Token(Kw_e kw, const SourceLocation &sl) : type(Kw), sl(sl), data(kw) {}

Lit Token::getValue() const { return std::get<::Lit>(data); }
std::string Token::getName() const { return std::get<std::string>(data); }
Kw_e Token::getKw() const { return std::get<Kw_e>(data); }

bool is_op(u8 ch) { return ch >= Not && ch <= And; }
bool is_ws(u8 ch) { return ch == Tab || ch == Space; }



static const std::map<ptr, Kw_e> kws{
    {hash("fn"), Fn},         {hash("for"), For},
    {hash("i8"), I8},         {hash("i16"), I16},
    {hash("i32"), I32},       {hash("i64"), I64},
    {hash("string"), String}, {hash("_"), Drop},
    {hash("if"), If},         {hash("import"), Import},
    {hash("extern"), Extern}, {hash("export"), Export},
    {hash("mod"), Module},    {hash("true"), True},
    {hash("false"), False},   {hash("bool"), Bool}};
Kw_e is_kw(ptr h) {
  auto k = kws.find(h);
  if (k != kws.end()) {
    return k->second;
  }
  return Unk;
}

Lit Lexer::nolit(const SourceLocation &s, bool f, int base) {
  llvm::StringRef sr(std::string(s.it, it));
  if (f) {
    double D = 0.0;
    sr.getAsDouble(D);
    return Lit(D);
  } else {
    int I = 0;
    sr.getAsInteger(base, I);
    return Lit(I);
  }
}

Lit Lexer::stringlit(std::string s) {
    return Lit(s);
}

Token &Token::operator=(const Token &other) {
  type = other.type;
  sl = other.sl;
  switch (other.type) {
  case Lit:
    data = other.data;
    break;
  case Kw:
    data = other.data;
    break;
  case Id:
    data = other.data;
    break;
  default:
    break;
  }
  return *this;
}

void Lexer::lex() {
  if(!can_iter()) {
    serror(Error_e::Unk,"Empty file");
  }
  while (can_iter()) {
    tokens.push_back(next());
  }
}
Token Lexer::lex_id_or_kw(SourceLocation& err_loc) {
  pop();
    while ((eq[peek()] == Letter || eq[peek()] == Number ||
            eq[peek()] == Underscore) &&
           can_iter()) {
      pop();
    }
    auto e = this->get_sourcelocation();
    auto s = std::string(err_loc.it, e.it);
    auto h = hash(s);
    auto k = is_kw(h);
    if (k != Unk) {
      return Token(k, e);
    }
    return Token(s, e);
}
Token Lexer::lex_number(SourceLocation& err_loc) {
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

Token Lexer::lex_string(SourceLocation& err_loc) {
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

Token Lexer::lex_newline(SourceLocation& err_loc) {
    pop();
    auto curr_indent = indent;
    while (eq[peek()] == Space) {
      pop();
      curr_indent++;
    }
    while (eq[peek()] == Tab) {
      pop();
      curr_indent++;
    }
    if (eq[peek()] == N) {
      return next();
    }
    if (indent < curr_indent) {
      indent = curr_indent;
      return Token(Token::Gi, this->get_sourcelocation());
    }
    if (indent > curr_indent) {
      indent = curr_indent;
      return Token(Token::Li, this->get_sourcelocation());
    }
    return Token(Token::N, err_loc);
}

Token Lexer::lex_dots(SourceLocation& err_loc) {
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

Token Lexer::lex_eq(SourceLocation& err_loc) {
    pop();
    if(eq[peek(1)]==Eq) {
      pop();
      return Token(Token::EqEq,err_loc);
    } else {
      return Token(Token::Eq,err_loc);
    }
}

Token Lexer::lex_mul(SourceLocation& err_loc) {
  pop();
  if(eq[peek(1)]==Eq) {
    pop();
    return Token(Token::MulEq,err_loc);
  } else {
    return Token(Token::Mul,err_loc);
  }
}

Token Lexer::lex_div(SourceLocation& err_loc) {
  //implement comments
  pop();
  if(eq[peek(1)]==Eq) {
    pop();
    return Token(Token::DivEq,err_loc);
  }
  return Token(Token::Div,err_loc);
}

Token Lexer::lex_not(SourceLocation& err_loc) {
  pop();
  if(eq[peek(1)]==Eq) {
    pop();
    return Token(Token::NotEq,err_loc);
  } else {
    return Token(Token::Not,err_loc);
  }
}

Token Lexer::lex_gt(SourceLocation& err_loc) {
  pop();
  auto n = eq[peek(1)];
  if(n == Eq) {
    pop();
    return Token(Token::GtEq,err_loc);
  }
  return Token(Token::Gt,err_loc);
}

Token Lexer::lex_lt(SourceLocation& err_loc) {
  pop();
  auto n = eq[peek(1)];
  if(n == Eq) {
    pop();
    return Token(Token::LtEq,err_loc);
  }
  return Token(Token::Lt,err_loc);
}

Token Lexer::lex_add(SourceLocation& err_loc) {
  pop();
  if(eq[peek(1)]==Eq) {
    pop();
    return Token(Token::AddEq,err_loc);
  }
  return Token(Token::Add,err_loc);
}

Token Lexer::lex_sub(SourceLocation& err_loc) {
  pop();
  if(eq[peek(1)]==Eq) {
    pop();
    return Token(Token::SubEq,err_loc);
  }
  return Token(Token::Sub,err_loc);
}

Token Lexer::lex_mod(SourceLocation& err_loc) {
  pop();
  if(eq[peek(1)]==Eq) {
    pop();
    return Token(Token::ModEq,err_loc);
  }
  return Token(Token::Mod,err_loc);
}

Token Lexer::lex_neg(SourceLocation& err_loc) {
  pop();
  if(eq[peek(1)]==Eq) {
    pop();
    return Token(Token::NegEq,err_loc);
  }
  return Token(Token::Neg,err_loc);
}

Lexer::Lexer(FSFile &file)
    : SourceLocation(file), file(file){};
Token Lexer::next() {
  while (eq[peek()] == Space) {
    pop();
  }
  SourceLocation err_loc = this->get_sourcelocation();

  u8 ch = eq[peek()];
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
    //Add commnet lexer here
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
    // serror(Error_e::UnkEsc, "Unknown escape character."/*, err_loc*/);
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
    : pos(0), indent(0), it(file.code.begin()), end(file.code.end()) {}

SourceLocation &SourceLocation::operator=(const SourceLocation &other) {
  pos = other.pos;
  indent = other.indent;
  it = other.it;
  end = other.end;
  return *this;
}

SourceLocation SourceLocation::get_sourcelocation() {
    return *this;
}
