#pragma once
#include <iostream>
#include <vector>
#include <map>
#define EqOffset 13;
/*
hash = offset_basis
for each octet_of_data to be hashed
hash = hash xor octet_of_data
hash = hash * FNV_prime
return hash

prime = 1099511628211;
offset=14695981039346656037;


*/

typedef uint64_t u64;
const static u64 offset = 14695981039346656037;
const static u64 prime = 1099511628211;

/*constexpr*/ u64 hash(const char const * s);

struct Range_t {
	enum Inf_e {
		RNot,
		RNoStart,
		RInf,
		RNoEnd
	};
	int start = 0, end = 0, step = 0;
	Inf_e infinite;
	Range_t(int start, int end, int step, Inf_e infinite) : start(start), end(end), step(step), infinite(infinite) {};
	Range_t(int start, int end, int step) :start(start), end(end), step(step), infinite(RNot) {};
	Range_t(Inf_e b = RInf) : infinite(b) {};
	Range_t(int start, int step) : start(start), step(step), infinite(RNoEnd) {};
	Range_t(int end, int step, Inf_e neg = RNoStart) : end(end), step(step), infinite(RNoStart) {};
};


enum _Lexem {
	N, //New Line
	Gi, // Greater Indent
	Li, //Less Indent

	Kw, //Keyword
	Id, //Identifier

	LitChar,
	LitString,
	LitBool,
	LitHex,
	LitFloat,
	LitInt,


	Lp,
	Rp,// )
	Lb,
	Rb,// ]
	Lc,
	Rc,// }

	Dot,
	Range,
	Elipsis,
	DoubleDot,
	DoubleDoubleDot,
	RightArrow, //->

				//Operators
				Add,
				Sub,
				Mul,
				Div,
				Mod,
				Neg,

				Not,
				And,
				Or,
				Xor,
				Less,
				Greater,
				Eq,

				// +13
				Addeq,
				Subeq,
				Muleq,
				Diveq,
				Modeq,
				Negeq,
				Noteq,
				Andeq,
				Oreq,
				Xoreq,
				Lesseq,
				Greatereq,
				Eqeq,

				AndAnd,
				OrOr,


};
enum Keyword {
	Fn,
	For,
	While,
	If,
	Else,
	Return,
	Yield,
	Sizeof,
	Typeof,
	Class, //struct
	Enum,
	Trait,
	Static,
	Cte,
	/*Cteq, //cte? */
	Inline,
	Pub,
	Priv,
	Namespace,
	Modulus,
	Use,
	Import,
	Export,
	Extern,
	/*Impl*/
	/*async*/
	/*await*/
};

const std::map<const u64, /*Keyword*/int> keywords = {
	{ hash("fn")			,		Fn },
{ hash("for")		,		For },
{ hash("while")		,		While },
{ hash("if")			,		If },
{ hash("else")		,		Else },
{ hash("return")		,		Return },
{ hash("Yield")		,		Yield },
{ hash("sizeof")		,		Sizeof },
{ hash("typeof")		,		Typeof },
{ hash("class")		,		Class },
{ hash("struct")		,		Class }, //Same as Class
{ hash("enum")		,		Enum },
{ hash("trait")		,		Trait },
{ hash("static")		,		Static },
{ hash("cte")		,		Cte },
{ hash("inline")		,		Inline },
{ hash("pub")		,		Pub },
{ hash("priv")		,		Priv },
{ hash("Namespace")	,		Namespace },
{ hash("mod")		,		Mod },
{ hash("use")		,		Use },
{ hash("import")		,		Import },
{ hash("Export")		,		Export },
{ hash("Extern")		,		Extern }



};


struct Lexem {
	_Lexem type;
	void* val;
	Lexem(_Lexem type, void* val) : type(type), val(val) {};
	~Lexem() {
		if (val)
			delete val;
	}
};



class Lexer {
	std::vector<Lexem> tokens;
	int i = 0, tcount = 0, ltcount = 0, len;
	std::string code;
	typedef void(Lexer::*lexfn)();



	void tabcount();
	void is_op();
	void is_kw_or_id();

	void is_char();
	void is_string();
	void is_hex();
	void is_num();
	void is_true();
	void is_false();
	bool get_int(int& res);
	void is_range();

	void is_lit();
public:
	Lexer() = default;
	Lexer(std::string code) : code(code), len(code.size()) {};
	void lex();
	std::vector<Lexem> GetTokens();

};