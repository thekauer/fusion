#include "lex.h"

/*constexpr*/ u64 hash(const char const * s) {
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





void Lexer::is_char()
{
	if (peek() == '\'') {
		pop();
		add(new Lit_Char(pop()));
		pop();
		std::cout << "works";
	}
}



void Lexer::lex() {
	while (can_iter()) {

	}
}


std::vector<token_t>& Lexer::GetTokens() {
	return tokens;
}
