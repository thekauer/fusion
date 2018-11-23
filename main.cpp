#include <iostream>
#include "lex.h"
#include <string>
#include "error.h"
#include "parser.h"




 

 void print_lexems(std::vector<token_t>& tokens) {

	for(const auto& t : tokens) {
		print_lexem(t->type);
		if (t->type == Kw) {
			std::cout << "\b::" << ((Key_Word*)t.get())->val << " ";
		}
		if (t->type == Id) {
			std::cout << "\b::" << ((I_d*)t.get())->val << " ";
		}

		if (t->type == LitString) {
			std::cout << "\b::\"" << ((Lit_String*)t.get())->val << "\" ";
		}
	}
 }

int main() {



	//const std::string test = "fn add(a : i32,b :i32)\n return a+b\n\nfn main()\n print(add(2,3))";
	const std::string test = "1+1";
	auto lexer= Lexer("main.fs",test);
	lexer.lex();
	print_lexems(lexer.GetTokens());
	std::cout << "\n\n";
	auto parser = Parser(lexer.GetTokens());
	parser.parse();


	std::cin.ignore();


	return 0;
}