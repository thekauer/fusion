#pragma once
#include "lex.h"


enum class Parse_e {
	FnDecl,
	FnCall,
	Var_Decl,
	For_in,
	For_times,
	If,
	If_in,
	Else,
	While,
	Loop, // for infinite loops
	Switch,
	Expr,
	Range,
	Generator,
};


enum class Op {
	//unary
	Not			,			//!
	Address		,			//&
	Pointer		,			//*
	Neg			,			//~
	Inc			,			//++
	Dec			,			//--

	//binary
	Assign		,			//a=b
	Add			,			//+
	Sub			,			//-
	Mul			,			//*
	Div			,			// /
	Mod			,			// %
	BitAnd		,			// a&b
	BitOr		,			// a|b
	BitXor		,			// a^b
	Less		,			// <
	Greater		,			// >
	Addeq		,			// +=
	Subeq		,			// -=
	Muleq		,			// *=
	Diveq		,			// /=
	Modeq		,			// %=
	Andeq		,			// &=
	Oreq		,			// |=
	Xoreq		,			// ^=
	And			,			// &&
	Or			,			// ||
	Eq			,			// ==
	Noteq		,			// !=
	Lesseq		,			// <=
	Greatereq	,			// >=
	Dot			,			// .
	Arrow		,			// ->



};


class Typed {
	bool is_const = false;
	I_d& type;
	Typed(I_d& type,bool is_const);
};

struct Parse_t {
	Parse_e type;
	Parse_t(Parse_e type);
};



class Parser {
	std::vector<token_t>& tokens;
public:
	Parser(std::vector<token_t>& tokens);
	void parse();
};