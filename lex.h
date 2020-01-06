#pragma once
#include "llvm/IR/Value.h"
#include <string>
#include "compatibility.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Constant.h"
#include "source.h"
#include "type.h"



ptr hash(const std::string& str);
class SourceLocation {
    private:
    char current,next,nextnext;
    public:
    const FSFile& file;
    int line,col,indent;
    std::string::const_iterator it,end;
    SourceLocation(const FSFile& file);
    INLINE constexpr char peek(const int n=0);
    INLINE char pop(int n=1);
    INLINE bool can_iter();
    private:
    INLINE char peek_();
    INLINE char peek_next();
    INLINE char peek_nextnext();
    INLINE char peek_nth(int n);
};




enum Kw_e : unsigned char {
    Unk,
    Fn
};



struct Token {
    enum Type :unsigned char {
    Id,
    Kw,

    N,
    Gi,
    Li,


    Char,
    String,
    I32,
    I16,
    I64,
    Bool,
    Float,
    Double,
    USize,
    U8,
    U16,
    U32,
    U64,

    Not=64,
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
    Rb,// ]
    Underscore,
    Xor, //^
    Lc, //{
    Rc, //}
    Or,
    Neg, //~
    Null,
    Space,
    Tab,
    And,

    } type;
    SourceLocation sl;
    
    union {
        Lit value;
        ptr hash;
        Kw_e kw;
    };
    Token(Type type,SourceLocation sl) : type(type),sl(sl){};
    Token(u8 c,SourceLocation sl) : type(static_cast<Type>(c)),sl(sl){};
    Token(Type type,Lit val,SourceLocation sl) :type(type),value(val),sl(sl){};
    Token(ptr hash,SourceLocation sl) :type(Id),hash(hash),sl(sl){};
    Token(Kw_e kw ,SourceLocation sl) : type(Kw),sl(sl){};
};



class Lexer :public SourceLocation{
public:
    Lexer(const FSFile& file);
    Token next();
private:
    char lex_escape(const char esc);
    Lit nolit(const SourceLocation& s,bool f,int base);
};