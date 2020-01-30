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
  Triangle, //^
  Lc,       //{
  Rc,       //}
  Or,
  Neg, //~
  Null,
  Space,
  Tab,
  And,
  // parsing required
  Quote,
  Apostrophe,
  Cr, // carrige return
  N,  // ascii 10 == 0xA
  Letter = 100,
  Number = 128
};
static const unsigned eq[128] = {Null,      0,      0,         0,
                                 0,         0,      0,         0,
                                 0,         0,      N,         0,
                                 0,         Cr,     0,         0,
                                 0,         0,      0,         0,
                                 0,         0,      0,         0,
                                 0,         0,      0,         0,
                                 0,         0,      0,         0,
                                 Space,     Not,    Quote,     Hashtag,
                                 0,         Mod,    And,       Apostrophe,
                                 Lp,        Rp,     Mul,       Add,
                                 Comma,     Sub,    Dot,       Div,
                                 Number,    Number, Number,    Number,
                                 Number,    Number, Number,    Number,
                                 Number,    Number, DoubleDot, SemiColon,
                                 Lt,        Eq,     Gt,        Questionmark,
                                 0,         Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Lb,
                                 Backslash, Rb,     Triangle,  Underscore,
                                 0,         Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Letter,
                                 Letter,    Letter, Letter,    Lc,
                                 Or,        Rc,     Neg};

Token::Token(Type type, const SourceLocation &sl) : type(type), sl(sl){};
Token::Token(u8 c, const SourceLocation &sl)
    : type(static_cast<Type>(c)), sl(sl) {}
Token::Token(llvm::Constant *val, const SourceLocation &sl)
    : type(Lit), sl(sl), data(val) {}
Token::Token(const std::string &str, const SourceLocation &sl)
    : type(Id), sl(sl), data(str) {}
Token::Token(Kw_e kw, const SourceLocation &sl) : type(Kw), sl(sl), data(kw) {}

llvm::Constant *Token::getValue() const {
  return std::get<llvm::Constant *>(data);
}
std::string Token::getName() const { return std::get<std::string>(data); }
Kw_e Token::getKw() const { return std::get<Kw_e>(data); }

bool is_op(u8 ch) { return ch >= Not && ch <= And; }
bool is_ws(u8 ch) { return ch == Tab || ch == Space; }

bool is_eol(u8 ch) { return ch == N || ch == Space; }

static const std::map<ptr, Kw_e> kws{
    {hash("fn"), Fn},         {hash("for"), For}, {hash("i8"), I8},
    {hash("i16"), I16},       {hash("i32"), I32}, {hash("i64"), I64},
    {hash("string"), String}, {hash("_"), Drop}};
Kw_e is_kw(ptr h) {
  auto k = kws.find(h);
  if (k != kws.end()) {
    return k->second;
  }
  return Unk;
}

llvm::Constant *Lexer::nolit(const SourceLocation &s, bool f, int base) {
  llvm::StringRef sr(std::string(s.it, it));
  if (f) {
    double D = 0.0;
    sr.getAsDouble(D);
    return llvm::ConstantFP::get(ctx.ctx, llvm::APFloat(D));
  } else {
    int I = 0;
    sr.getAsInteger(base, I);
    return llvm::ConstantInt::get(ctx.getI32(), llvm::APInt(32, I, true));
  }
}

llvm::Constant *Lexer::stringlit(std::string s) {
  llvm::Type *i8ty = llvm::IntegerType::getInt8Ty(ctx.ctx);
  llvm::ArrayType *sty = llvm::ArrayType::get(i8ty, s.size() + 1);
  std::vector<llvm::Constant *> vals;
  llvm::GlobalVariable *gstr = new llvm::GlobalVariable(
      *ctx.mod, sty, true, llvm::GlobalValue::PrivateLinkage, 0, "str");
  gstr->setAlignment(1);
  llvm::Constant *cstr = llvm::ConstantDataArray::getString(ctx.ctx, s, true);
  gstr->setInitializer(cstr);
  return llvm::ConstantExpr::getBitCast(gstr, ctx.getI8()->getPointerTo());
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
  while (can_iter()) {
    tokens.push_back(next());
  }
}

Lexer::Lexer(FSFile &file, FusionCtx &ctx)
    : SourceLocation(file), file(file), ctx(ctx){};
Token Lexer::next() {
  while (eq[peek()] == Space) {
    pop();
  }
  SourceLocation err_loc = sl_cast(this);

  u8 ch = eq[peek()];
  switch (ch) {
  case Letter: {
    if (eq[peek()] == Letter || eq[peek()] == Underscore) {
      pop();
    }
    while ((eq[peek()] == Letter || eq[peek()] == Number ||
            eq[peek()] == Underscore) &&
           can_iter()) {
      pop();
    }
    auto e = sl_cast(this);
    auto s = std::string(err_loc.it, e.it);
    auto h = hash(s);
    auto k = is_kw(h);
    if (k != Unk) {
      return Token(k, e);
    }
    return Token(s, e);
  }
  case Number: {
    // Handle base
    int base = 10;
    while (eq[peek()] == Number) {
      pop();
    }
    bool is_float = false;
    if (eq[peek()] == Dot) {
      pop();
      is_float = true;
    }
    while (eq[peek()] == Number) {
      pop();
    }
    // handle suffix
    if (is_float) {
      return Token(nolit(err_loc, is_float, 10), err_loc);
    } else {
      return Token(nolit(err_loc, is_float, base), err_loc);
    }
  }
  case Quote: {
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
  case N: {
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
    if (indent < curr_indent) {
      indent = curr_indent;
      return Token(Token::Gi, sl_cast(this));
    }
    if (indent > curr_indent) {
      indent = curr_indent;
      return Token(Token::Li, sl_cast(this));
    }
    return Token(Token::N, err_loc);
  }

  default:
    break;
  }
  if (is_op(ch)) {
    pop(); // pop the operator, it is stored in ch
    switch (eq[peek()]) {
    case Div: {
      auto n = eq[peek()];
      if (n == Div) {
        pop(); // pop second /
        // TODO: check for third /
        while (can_iter() && !is_eol(eq[peek()]))
          pop();
        return next();
      }
      if (n == Mul) {
        pop(); // pop the *
        // implement Multi COmment in Multi Comment
        while (can_iter() && eq[peek()] != Div) {
          while (can_iter() && eq[peek()] != Mul)
            pop();
        }
      }
      break;
    }
    default:
      break;
    }
    return Token(ch, sl_cast(this));
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
    Error::UnkEsc(file, err_loc, esc);
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
