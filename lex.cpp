#include "lex.h"

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
		add(new Lexem(Gi));
	}
	if(tcount < ltcount) {
		add(new Lexem(Li));
	}
	ltcount = tcount;
	tcount =0;

}

void Lexer::is_char()
{
	if (peek() == '\'') {
		pop();
		add(new Lit_Char(pop()));
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
		/* FIX MEE PLS*/
		std::map<u64,int>::const_iterator* res = keywords.find(hash);
		if(res) {
			add(new Key_Word(res->second));
		} else {
			add(new I_d(hash));
		}

	}
}



void Lexer::is_num() {
	std::string s="";
	bool f=false;


	/*0b810100 BUG CAUSES infinite loop */
	if(peek()=='0')  {
		if(peek(1)=='x') {
			pop(2);
			while(isxdigit(peek())) {
				s+=pop();
			}
			add(new Lit_Hex(strtol(s.c_str(),NULL,16)));
			return;
		}
		if(peek(1)=='b') {
			pop(2);
			while(peek()=='0' || peek()=='1' || peek()=='_') {
				if(peek()=='_')pop();
				s+=pop();
			}
			add(new Lit_Int(strtol(s.c_str(),NULL,2)));
			return;

		}
	}

	//1. is of type int not float!!!

	if(peek()=='-' || isdigit(peek())) {
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
		add(new Lit_Float(atof(s.c_str())));
	else
		add(new Lit_Int(atoi(s.c_str())));
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
		add(new Lit_Bool(true));
	} 
	if(peek()=='f' &&peek(1)=='a'&&peek(2)=='l'&&peek(3)=='s'&&peek(4)=='e'&&!isalpha(peek(5))) {
		pop(5);
		add(new Lit_Bool(false));
	}
}

void Lexer::consume_newline() {
	while(peek()=='\n') {
		pop();
		add(new Lexem(N));
	}
}

void Lexer::lex() {
	while(can_iter()) {
		
		consume_newline();
		tabcount();
		is_char();
		is_bool();
		is_num();
		is_kw_or_id();
		consume_space();
		
	}

		
	
}


std::vector<token_t>& Lexer::GetTokens() {
	return tokens;
}

std::vector<Error>& Lexer::GetErrors() {
	return errors;
}