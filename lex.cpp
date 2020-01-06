#include "lex.h"
#include <map>

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
                                Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Letter,Lc,Or,Rc,Neg};

static bool is_op(u8 ch) {
    return ch>=Not && ch<=And;
}
static bool is_ws(u8 ch) {
    return ch==Tab || ch==Space;
}
static SourceLocation sl_cast(Lexer* l) {
    return *reinterpret_cast<SourceLocation*>(l);
}

static const std::map<ptr,Kw_e> kws {
        {hash("fn"),Fn}
    };
static Kw_e is_kw(ptr h) {
    auto k = kws.find(h);
    if(k!=kws.cend()) {
        return k->second;
    }
    return Unk;
}

Lit Lexer::nolit(const SourceLocation& s,bool f,int base) {
    llvm::StringRef sr(std::string(s.it,it));
    int I;
    double D;
    if(f) {
        sr.getAsDouble(D);
        auto dt = IntegralType(Double,false,Copy);
        return Lit(dt,static_cast<ptr>(D));
    } else {
        sr.getAsInteger(base,I);
        auto ity =IntegralType(I32,false,Copy);
        return Lit(ity,static_cast<ptr>(I));
    }
}

Lexer::Lexer(const FSFile& file) : SourceLocation(file){};
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
        auto s = hash(std::string(err_loc.it,e.it));
        auto k = is_kw(s);
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
            return Token(Token::Double,nolit(err_loc,is_float,10),err_loc);
        } else {
            return Token(Token::I32,nolit(err_loc,is_float,base),err_loc);
        }


    }
    case N:{
        line++;
        pop();
        while(eq[peek()]==Space) {
            if(eq[pop()]==Tab);//error;
        }
        while(eq[peek()]==Tab) {
            if(eq[pop()]==Space);//error
        }
        if(indent>err_loc.indent) return Token(Token::Gi,sl_cast(this));
        if(indent<err_loc.indent) return Token(Token::Li,sl_cast(this));
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




INLINE constexpr char SourceLocation::peek(const int n) {
    switch (n)
    {
    case 0:
        return peek_();
        break;
    case 1:
        return peek_next();
        break;
    case 2:
        return peek_nextnext();
        break;
    
    default:
        return peek_nth(n);
    }
}

INLINE char SourceLocation::peek_nth(int n) {
    return *it+n;
}

INLINE char SourceLocation::peek_() {
    return current;
}

INLINE char SourceLocation::peek_next() {
    return next;
}

INLINE char SourceLocation::peek_nextnext() {
    return nextnext;
}

INLINE char SourceLocation::pop(int n) {
    char r=peek(n-1);
    it+=n;
    col++;
    prefetch((const char*)&it);
    current=next;
    next=nextnext;
    if (can_iter()) {
        nextnext = *it;
    }
    return r;
}


INLINE bool SourceLocation::can_iter() {
    return it!=end;
}





SourceLocation::SourceLocation(const FSFile& file) : file(file),line(1),col(1),indent(0),it(file.code.begin()),end(file.code.end()) {
    current=*it;
    next=*(it++);
    nextnext=*(it++);

}
