#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include "lexiter.h"
#include <string.h>
#include "error.h"
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

/*constexpr*/ u64 hash(const char * s);

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

	//ops start
	Lp,
	Rp,// )
	Lb,
	Rb,// ]
	Lc,
	Rc,// }

	Comma,
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
				AddAdd,
				SubSub


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
	unsigned int column, lineno;
	Lexem(_Lexem type,unsigned int col,unsigned int line) : type(type),column(col),lineno(line) {};
	
};
struct Lit_Int :public Lexem {
	int val;
	Lit_Int(int, unsigned int col, unsigned int line) : Lexem(LitInt,col,line), val(val) {};
};
struct Lit_Char: public Lexem {
	char val;
	Lit_Char(char val, unsigned int col, unsigned int line) : Lexem(LitChar,col,line), val(val) {};
};
struct Lit_Hex : public Lexem {
	long val;
	Lit_Hex(long val, unsigned int col, unsigned int line) : Lexem(LitHex,col,line), val(val) {};
};
struct Lit_Bool : public Lexem {
	bool val;
	Lit_Bool(bool val, unsigned int col, unsigned int line) : Lexem(LitBool,col,line), val(val) {};
};
struct Lit_String : public Lexem {
	std::string val;
	Lit_String(std::string val, unsigned int col, unsigned int line) : Lexem(LitString,col,line), val(val) {};
};
struct Lit_Float : public Lexem {
	double val;
	Lit_Float(double val, unsigned int col, unsigned int line) : Lexem(LitFloat,col,line), val(val) {};
};
struct Key_Word : public Lexem {
	Keyword val;
	Key_Word(Keyword val, unsigned int col, unsigned int line) : Lexem(Kw,col,line), val(val) {};
};
struct I_d : public Lexem {
	u64 val;
	I_d(u64 val, unsigned int col, unsigned int line) : Lexem(Id,col,line), val(val) {};
};


	typedef std::unique_ptr<Lexem> token_t;

void print_lexem(_Lexem);


class Lexer : Iterator {
public:
	std::vector<token_t> tokens;
	int tcount = 0, ltcount = 0;
	std::vector<Error> errors;
	std::string fname;
	
	void tabcount();
	void is_char();
	void consume_newline();
	void consume_space();
	void is_bool();
	void is_kw_or_id();
	void is_string();
	void is_op();

public:
	void is_num();


public:
	Lexer() = default;
	Lexer(std::string fname,std::string code);
	void lex();
	std::vector<token_t>& GetTokens();
	std::vector<Error>& GetErrors();

};