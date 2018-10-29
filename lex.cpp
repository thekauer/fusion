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



void Lexer::is_num() {
	std::string s="";
	bool f=false;

	if(peek()=='-') s+=pop();
	while(isdigit(peek()) || (!f && peek()=='.')) {
		/*TODO: FIX 1.. bug  */
		if(peek()=='.' && !f) f=true;
		s+=pop();
	}
	if(f)
		add(new Lit_Float(atof(s.c_str())));
	else
		add(new Lit_Int(atoi(s.c_str())));


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
		consume_space();
		
	}

		
	
}


std::vector<token_t>& Lexer::GetTokens() {
	return tokens;
}

std::vector<Error>& Lexer::GetErrors() {
	return errors;
}