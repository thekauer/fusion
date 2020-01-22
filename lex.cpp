#include "lex.h"
#include <map>
#include "llvm/IR/Constants.h"
ptr hash(const std::string& str){
    
	u32 h = 123456;
	u32 len = str.size();
	const u8* key = (const u8*)(&str[0]);
	if (len > 3) {
		const u32* key_x4 = (const uint32_t*)(&str[0]);
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
		key = (const u8*)key_x4;
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
    Triangle, //^
    Lc, //{
    Rc, //}
    Or,
    Neg, //~
    Null,
    Space,
    Tab,
    And,
    //parsing required
    Quote,
    Apostrophe,
    Cr, // carrige return
    N, //ascii 10 == 0xA
    Letter=100,
    Number=128
};
static const unsigned eq[128] = {Null,0,0,0,0,0,0,0,0,0,N,0,0,Cr,0,0,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                Space,Not,Quote,Hashtag,0,Mod,And,Apostrophe,Lp,Rp,Mul,Add,Comma,Sub,Dot,Div,
                                Number,Number,Number,Number,Number,Number,Number,Number,Number,Number,DoubleDot,SemiColon,Lt,Eq,Gt,Questionmark,
                                0,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,
                                Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Lb,Backslash,Rb,Triangle,Underscore,
                                0,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,
                                Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Lc,Or,Rc,Neg
};




Token::Token(Type type,const SourceLocation& sl) : type(type),sl(sl){};
Token::Token(u8 c,const SourceLocation& sl) : type(static_cast<Type>(c)),sl(sl){}
Token::Token(llvm::Constant* val,const SourceLocation& sl) : type(Lit),sl(sl),data(val){}
Token::Token(const std::string& str,const SourceLocation& sl) : type(Id),data(str),sl(sl){}
Token::Token(Kw_e kw,const SourceLocation& sl): type(Kw),sl(sl){}

llvm::Constant* Token::getValue() const{
    return std::get<llvm::Constant*>(data);
}
std::string Token::getName() const {
    return std::get<std::string>(data);
}
Kw_e Token::getKw() const{
    return std::get<Kw_e>(data);
}






bool is_op(u8 ch) {
    return ch>=Not && ch<=And;
}
static bool is_ws(u8 ch) {
    return ch==Tab || ch==Space;
}


static const std::map<ptr,Kw_e> kws {
        {hash("fn"),Fn}
    };
static Kw_e is_kw(ptr h) {
    auto k = kws.find(h);
    if(k!=kws.end()) {
        return k->second;
    }
    return Unk;
}

llvm::Constant* Lexer::nolit(const SourceLocation& s,bool f,int base) {
    llvm::StringRef sr(std::string(s.it,it));
    int I;
    double D;
    if(f) {
        sr.getAsDouble(D);
        
        return llvm::ConstantInt::get(
            llvm::IntegerType::getInt32Ty(ctx.ctx),
            llvm::APInt(32,I,true)
        );
    } else {
        sr.getAsInteger(base,I);
        return llvm::ConstantFP::get(ctx.ctx,llvm::APFloat(D));
    }
}

llvm::Constant* Lexer::stringlit(std::string s) {
    llvm::Type* i8ty = llvm::IntegerType::getInt8Ty(ctx.ctx);
    llvm::ArrayType* sty = llvm::ArrayType::get(i8ty,s.size()+1);
    std::vector<llvm::Constant*> vals;
    for(char c : s) {
        llvm::Constant* cc = llvm::ConstantInt::get(i8ty,c,true);
        vals.push_back(cc);
    }
    return llvm::ConstantArray::get(sty,vals);
}

Token& Token::operator=(const Token& other) {
    type=other.type;
    sl=other.sl;
    switch (other.type)
    {
    case Lit:
        data=other.data;
        break;
    case Kw:
        data=other.data;
    case Id:
        data=other.data;
    
    default:
        break;
    }
    return *this;

}

void Lexer::lex() {
    while(can_iter()) {
        tokens.push_back(next());
    }
}

Lexer::Lexer(FSFile& file,FusionCtx& ctx) : SourceLocation(file),ctx(ctx){};
Token Lexer::next() {
    SourceLocation err_loc = sl_cast(this);
    while(eq[peek()]==Space) {
        pop();
    }


    u8 ch = eq[peek()];
    switch (ch)
    {
    case Letter: {
        if(eq[peek()]==Letter || eq[peek()]==Underscore) {
            pop();
        }
        while((eq[peek()]==Letter || eq[peek()]==Number 
        || eq[peek()]==Underscore)
        &&can_iter()){pop();}
        auto e = sl_cast(this);
        auto s =std::string(err_loc.it,e.it);
        auto h = hash(s);
        auto k = is_kw(h);
        if(k!=Unk) {
            return Token(k,e);
        }
        return Token(s,e);
    }
    case Number: {
        //Handle base
        int base=10;
        while(eq[peek()]==Number) {
            pop();
        }
        bool is_float=false;
        if(eq[peek()]==Dot) {
            pop();
            is_float=true;
        }
        while(eq[peek()]==Number) {
            pop();
        }
        //handle suffix
        if(is_float) {
            return Token(nolit(err_loc,is_float,10),err_loc);
        } else {
            return Token(nolit(err_loc,is_float,base),err_loc);
        }


    }
    case Quote: {
        pop(); // pop the quote
        std::string buff;
        while(eq[peek()]!=Quote) {
            buff+=pop();
        }
        pop(); // pop the ending quote
        
        return Token(stringlit(buff),err_loc);
    }
    case N:{
        line++;
        col=1;
        pop();
        while(eq[peek()]==Space) {
            if(eq[pop()]==Tab);//error;
        }
        while(eq[peek()]==Tab) {
            if(eq[pop()]==Space);//error
        }
        if(indent<col) return Token(Token::Gi,sl_cast(this));
        if(indent>col) return Token(Token::Li,sl_cast(this));
        return Token(Token::N,err_loc);
    }
       
    
    default:
        break;
    }
    if(is_op(ch)) {
        pop();
        return Token(ch,sl_cast(this));
    }

}
char Lexer::lex_escape(const char esc) {
    auto err_loc = *reinterpret_cast<SourceLocation*>(this);
     switch (esc)
            {
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
                //cerror(ERR_UNKNOWN_ESCAPE,err_loc,"Unknown escape character.");
                break;
            }
}




INLINE  char SourceLocation::peek(const int n) {
    return *it;
}

INLINE char SourceLocation::pop() {
    char r=*it;
    col++;
    prefetch(&it);
    ++it;
    return r;
}


INLINE bool SourceLocation::can_iter() {
    return it!=end;
}


void Lexer::test() {
    while(can_iter()) {
        std::cout<<pop();
    }
}


SourceLocation::SourceLocation(FSFile& file) : file(file),line(1),col(1),indent(0),it(file.code.begin()),end(file.code.end()) {
}

SourceLocation& SourceLocation::operator=(const SourceLocation& other) {
    file=other.file;
    line=other.line;
    col=other.col;
    indent=other.indent;
    it=other.it;
    end=other.end;
    return *this;
}
