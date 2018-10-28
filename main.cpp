#include <iostream>
#include "lex.h"






int main() {
	const std::string test = "fn add(a : i32,b :i32)\n return a+b\n\nreturn fn main()\n print(add(2,3))";
	Lexer l(test);
	l.lex();
	for (const auto i : l.GetTokens()) {
		std::cout << i.type << " ";
	}
	
	std::cin.ignore();


	return 0;
}
