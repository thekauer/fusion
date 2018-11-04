#include "lex.h"
#define BUG_FIX " \n\n"

/*constexpr*/ u64 hash(const char * s) {
	u64 hash = offset;
	for (size_t i = 0; i < strlen(s); i++) {
		hash ^= s[i];
		hash *= prime;
	}
	return hash;
}



//#define add(e) tokens.push_back(std::unique_ptr<Lexem>( std::move((e)) ))
//#define add(e) tokens.push_back(std::move(std::unique_ptr<Lexem>((e))))
#define add(e) tokens.push_back(std::move(token_t((e))))



void Lexer::tabcount() {
	while(peek()=='\t' || peek()==' ') {
		if(peek()=='\t') {
			tcount+=4;
		} else {
			tcount+=1;
		}
		pop();
	} 
	if(tcount > ltcount) {
		add(new Lexem(Gi,col,line));
	}
	if(tcount < ltcount) {
		add(new Lexem(Li, col, line));
	}
	ltcount = tcount;
	tcount =0;

}

void Lexer::is_char()
{
	if (peek() == '\'') {
		pop();
		add(new Lit_Char(pop(), col, line));
		if(pop()!='\'') {
			errors.push_back(Error(fname,line,col,Error::Err,"Expected a closing \' in char literal."));
		}
		
	}
}


void Lexer::is_kw_or_id() {
	u64 hash=offset;
	if(isalpha(peek())) {
		hash^=pop();
		hash*=prime;
		while(isalpha(peek()) || isdigit(peek())) {
			hash^=pop();
			hash*=prime;
		}
		auto res = keywords.find(hash);
		if(res!=keywords.cend()) {
			add(new Key_Word((Keyword)res->second, col, line));
		} else {
			add(new I_d(hash, col, line));
		}

	}
}



void Lexer::is_num() {
	std::string s="";
	bool f=false;


	/*0b810100 BUG CAUSES infinite loop  (non 0-1)*/
	if(peek()=='0')  {
		if(peek(1)=='x') {
			pop(2);
			while(isxdigit(peek())) {
				s+=pop();
			}
			add(new Lit_Hex(strtol(s.c_str(),NULL,16), col, line));
			return;
		}
		if(peek(1)=='b') {
			pop(2);
			while(peek()=='0' || peek()=='1' || peek()=='_') {
				if(peek()=='_')pop();
				s+=pop();
			}
			add(new Lit_Int(strtol(s.c_str(),NULL,2), col, line));
			return;

		}
	}

	//1. is of type int not float!!!

	if((peek()=='-' && isdigit(peek(1))) || isdigit(peek())) {
		s+=pop();
	while(isdigit(peek()) || (!f && peek()=='.')) {
		if(f && peek()=='.') break;
		if(peek()=='.' && !f ) {
			if(isdigit(peek(1))) f=true;
			else break;
		}
		s+=pop();
	}
	if(f)
		add(new Lit_Float(atof(s.c_str()), col, line));
	else
		add(new Lit_Int(atoi(s.c_str()), col, line));
}




}

void Lexer::is_op() {
	switch (peek())
	{
	case '(':add(new Lexem(Lp,col,line)); pop();break;
	case '[':add(new Lexem(Lb,col,line)); pop();break;
	case '{':add(new Lexem(Lc,col,line)); pop();break;
	case ')':add(new Lexem(Rp,col,line)); pop();break;
	case ']':add(new Lexem(Rb,col,line)); pop();break;
	case '}':add(new Lexem(Rc,col,line)); pop();break;

	case '.':
		if (peek(1) == '.') {
			if (peek(2) == '.') { add(new Lexem(Elipsis, col, line)); pop(3); }
			else { add(new Lexem(Range, col, line)); pop(2); }
		}
		else { add(new Lexem(Dot, col, line)); pop(); }
		break;

	case ':':
		if (peek(1) == ':') {
			add(new Lexem(DoubleDoubleDot, col, line)); pop(2);}
		else { add(new Lexem(DoubleDot, col, line)); pop(); }
		break;

	case ',': add(new Lexem(Comma, col, line)); pop(); break;





	case '+':
		if (peek(1) == '=') { add(new Lexem(Addeq, col, line)); pop(2); break; }
		if (peek(1) == '+') {
			add(new Lexem(AddAdd, col, line)); pop(2); break;
		}
		else {
			add(new Lexem(Add, col, line)); pop(); break;
		}

	case '-':
		if (peek(1) == '>') {
			add(new Lexem(RightArrow, col, line)); pop(2); break;
		}
		if (peek(1) == '-') {
			add(new Lexem(SubSub, col, line)); pop(2); break;
		}
		if (peek(1) == '=') {
			add(new Lexem(Subeq, col, line)); pop(2); break;
		}
		else { add(new Lexem(Sub, col, line)); pop(); break; }
	case '/':
		if (peek(1) == '=') { add(new Lexem(Diveq, col, line)); pop(2); break; }
		else { add(new Lexem(Div, col, line)); pop(); break; }
	case '*':
		if (peek(1) == '=') {
			add(new Lexem(Muleq, col, line)); pop(2); break;
		}
		else {
			add(new Lexem(Mul, col, line)); pop(); break;
		}
	case '%':
		if (peek(1) == '=') {
			add(new Lexem(Modeq, col, line)); pop(2); break;
		}
		else {
			add(new Lexem(Mod, col, line)); pop(); break;
		}
	case '~':
		if (peek(1) == '=') {
			add(new Lexem(Negeq, col, line)); pop(2); break;
		}
		else {
			add(new Lexem(Neg, col, line)); pop(); break;
		}

	case '!':
		if (peek(1) == '=') {
			add(new Lexem(Noteq, col, line)); pop(2); break;
		}
		else {
			add(new Lexem(Not, col, line)); pop(); break;
		}
	case '&':
		if (peek(1) == '=') { add(new Lexem(Andeq, col, line)); pop(2); break; }
		if (peek(1) == '&') {
			add(new Lexem(AndAnd, col, line)); pop(2); break;
		}
		else {
			add(new Lexem(And, col, line)); pop(); break;
		}
	case '|':
		if (peek(1) == '=') { add(new Lexem(Oreq, col, line)); pop(2); break; }
		if (peek(1) == '|') {
			add(new Lexem(OrOr, col, line)); pop(2); break;
		}
		else {
			add(new Lexem(Or, col, line)); pop(); break;
		}
	case '^':
		if (peek(1) == '=') {
			add(new Lexem(Xoreq, col, line)); pop(2); break;
		}
		else {
			add(new Lexem(Xor, col, line)); pop(); break;
		}
	case '<':
		if (peek(1) == '=') {
			add(new Lexem(Lesseq, col, line)); pop(2); break;
		}
		else {
			add(new Lexem(Less, col, line)); pop(); break;
		}
	case '>':
		if (peek(1) == '=') {
			add(new Lexem(Greatereq, col, line)); pop(2); break;
		}
		else {
			add(new Lexem(Greater, col, line)); pop(); break;
		}
	case '=':
		if (peek(1) == '=') {
			add(new Lexem(Eqeq, col, line)); pop(2); break;
		}
		else{
			add(new Lexem(Eq, col, line)); pop(2); break;
		}

		break;





	default:
		break;
	}
}



void Lexer::is_string() {
	//WHEN Testing " is \\\" !!!!!
	if (peek() == '\"') {
		std::string buff = "";
		pop();
		while (peek() != '\"' && can_iter()) {
			if (peek() == '\\') {
				pop();
				switch (pop())
				{
				case 'n':
					buff += '\n';
					break;
				case '\"':
					buff += '\"';
					break;
				case 't':
					buff += '\t';
					break;
				case 'b':
					buff += '\b';
					break;
				case 'r':
					buff += '\r';
					break;
				case '?':
					buff += '\?';
					break;
				case '\'':
					buff += '\'';
					break;

				default:
					break;
				}

			}
				buff += pop();

		}
		pop();
		add(new Lit_String(buff, col, line));
	}
}


void Lexer::consume_space() {
	while(peek()=='\t' || peek()==' ') {
		pop();
	}
}


void Lexer::is_bool() {
	if(peek()=='t' && peek(1)=='r' && peek(2)=='u'
		&& peek(3) =='e' && !isalpha(peek(4))) {
		pop(4);
		add(new Lit_Bool(true, col, line));
	} 
	if(peek()=='f' &&peek(1)=='a'&&peek(2)=='l'&&peek(3)=='s'&&peek(4)=='e'&&!isalpha(peek(5))) {
		pop(5);
		add(new Lit_Bool(false, col, line));
	}
}

void Lexer::consume_newline() {

	while(peek()=='\n') {
		pop();
		add(new Lexem(N, col, line));
		tabcount();
	}

}

Lexer::Lexer(std::string fname, std::string code) : fname(fname), Iterator(code+BUG_FIX) 
{
	
}

void Lexer::lex() {
	while(can_iter()) {
		
		consume_newline();
		//tabcount();
		is_char();
		is_string();
		is_bool();
		is_num();
		is_kw_or_id();
		is_op();
		consume_space();
		
	}

		
	
}


std::vector<token_t>& Lexer::GetTokens() {
	return tokens;
}

std::vector<Error>& Lexer::GetErrors() {
	return errors;
}