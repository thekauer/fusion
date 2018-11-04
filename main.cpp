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
			strify(AddAdd)				,
			strify(SubSub)				,

		};
		std::cout << lexems[l] << " ";
	}



int main() {



	//const std::string test = "fn add(a : i32,b :i32)\n return a+b\n\nfn main()\n print(add(2,3))";
	const std::string test = "fn a1()\n if(true)\n  if(false)\n   1\n  else\n   2\n else\n  3";
	auto lexer= Lexer("main.fs",test);
	lexer.lex();

	for(const auto& t : lexer.GetTokens()) {
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


	std::cin.ignore();


	return 0;
}