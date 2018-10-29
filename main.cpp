#include <iostream>
#include "lex.h"
#include <string>
#include "error.h"
#define strify(a) #a



 void print_lexem(_Lexem l) {
		const std::string
 lexems[] = {
		    strify(N)					,
			strify(Gi)					, 
			strify(Li)					, 
			strify(Kw)					, 
			strify(Id)					,
			strify(LitChar)				,
			strify(LitString)			,
			strify(LitBool)				,
			strify(LitHex)				,
			strify(LitFloat)				,
			strify(LitInt)				,
			strify(Lp)					,
			strify(Rp)					,
			strify(Lb)					,
			strify(Rb)					,
			strify(Lc)					,
			strify(Rc)					,
			strify(Comma)				,
			strify(Dot)					,
			strify(Range)				,
			strify(Elipsis)				,
			strify(DoubleDot)			,
			strify(DoubleDoubleDot)		,
			strify(RightArrow)			, 
			strify(Add)					,
			strify(Sub)					,
			strify(Mul)					,
			strify(Div)					,
			strify(Mod)					,
			strify(Neg)					,
			strify(Not)					,
			strify(And)					,
			strify(Or)					,
			strify(Xor)					,
			strify(Less)					,
			strify(Greater)				,
			strify(Eq)					,
			strify(Addeq)				,
			strify(Subeq)				,
			strify(Muleq)				,
			strify(Diveq)				,
			strify(Modeq)				,
			strify(Negeq)				,
			strify(Noteq)				,
			strify(Andeq)				,
			strify(Oreq)					,
			strify(Xoreq)				,
			strify(Lesseq)				,
			strify(Greatereq)			,
			strify(Eqeq)					,
			strify(AndAnd)				,
			strify(OrOr)					,

		};
		std::cout << lexems[l] << " ";
	}



int main() {



	//const std::string test = "fn add(a : i32,b :i32)\n return a+b\n\nfn main()\n print(add(2,3))";
	const std::string test = "\'a\'";
	auto lexer= Lexer(test);
	auto err = Error("main.cpp", 1, 1, Error::Err, "Nothin written");
	err.print();


	std::cin.ignore();


	return 0;
}