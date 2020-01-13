#pragma once
#include "llvm/IR/Value.h"
#include <string>
#include "compatibility.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/LLVMContext.h"
#include "source.h"
#include "context.h"



ptr hash(const std::string& str);
class SourceLocation {
    public:
    FSFile& file;
    int line,col,indent;
    std::string::const_iterator it,end;
    SourceLocation(FSFile& file);
    INLINE char peek(const int n=0);
    INLINE char pop();
    INLINE bool can_iter();
    SourceLocation& operator=(const SourceLocation& other);
};

bool is_op(u8 ch);


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


    Lit,

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
        llvm::Constant* value;
        llvm::StringRef str;
        Kw_e kw;
    };
    Token(Type type,SourceLocation sl) : type(type),sl(sl){};
    Token(u8 c,SourceLocation sl) : type(static_cast<Type>(c)),sl(sl){};
    Token(llvm::Constant* val,SourceLocation sl) :type(Lit),value(val),sl(sl){};
    Token(llvm::StringRef str,SourceLocation sl) :type(Id),str(str),sl(sl){};
    Token(Kw_e kw ,SourceLocation sl) : type(Kw),sl(sl){};
    Token& operator=(const Token& other);
};
template<typename T>
static SourceLocation sl_cast(T* l) {
    return *reinterpret_cast<SourceLocation*>(l);
}


class Lexer :public SourceLocation{
public:
    Lexer(FSFile& file);
    Token next();
    void lex();
    void test();
    std::vector<Token> tokens;
private:
    char lex_escape(const char esc);
    llvm::Constant* nolit(const SourceLocation& s,bool f,int base);
};