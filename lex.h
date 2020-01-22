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
#include <variant>


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

    Token(Type type,const SourceLocation& sl);
    Token(u8 c,const SourceLocation& sl);
    Token(llvm::Constant* val,const SourceLocation& sl);
    Token(const std::string& str,const SourceLocation& sl);
    Token(Kw_e kw,const SourceLocation& sl);
    Token& operator=(const Token& other);
    llvm::Constant* getValue() const;
    std::string getName() const;
    Kw_e getKw() const;
private:
    std::variant<llvm::Constant*,std::string,Kw_e> data;
};
template<typename T>
static SourceLocation sl_cast(T* l) {
    return *reinterpret_cast<SourceLocation*>(l);
}


class Lexer :public SourceLocation {
public:
    Lexer(FSFile& file,FusionCtx& ctx);
    Token next();
    void lex();
    void test();
    std::vector<Token> tokens;
private:
    FusionCtx& ctx;
    char lex_escape(const char esc);
    llvm::Constant* nolit(const SourceLocation& s,bool f,int base);
    llvm::Constant* stringlit(std::string s);
};
