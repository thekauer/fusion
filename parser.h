#pragma once
#include "lex.h"

typedef uint8_t u8;

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







struct Ast {
	enum type_ {
		Expr,
		Typed,
		FnDecl
	};
	type_ type;
	Ast(type_ type);
};


struct Expr : Ast {
	enum type_ {
		Unary,
		Binary,
		Paren
	};
	type_ type;
	Expr(type_ type);
};


struct UnaryExpr : Expr {
	Op op;
	std::unique_ptr<Ast> val;
	UnaryExpr(Op op);
};
struct BinaryExpr : Expr {
	Op op;
	std::unique_ptr<Ast> left,right;
	BinaryExpr(Op op);
};
struct ParenExpr : Expr {
	std::vector<std::unique_ptr<Ast>> childs;
	ParenExpr();
};

struct Typed : Ast {
	enum type_ {
		Fncall,
		Lit,
		Var
	} type;

	Typed(type_ type);
};

struct Lit : Typed {
	std::unique_ptr<Lexem> lit;
	Lit(std::unique_ptr<Lexem> lit);
};


struct FnDecl : Ast {
	u64 name;
	struct FnMods {
		u8 Cte :1;
		u8 CteQ:1;
		u8 Inline:1;
		u8 Static :1;
		u8 Public :1;
		u8 Virtual:1;
		u8 Override:1;
		u8 Extern :1;
	} mods;
	struct FnGenArg {
		u64 name;
		FnGenArg(u64 name);
	};
	std::vector<FnGenArg> gen_args;
	struct FnArg {
		u64 name;
		u64 type; // FIX ME
		FnArg(u64 name);
		FnArg(u64 name,u64 type);
	};
	//RETURN TYPE
	std::vector<std::unique_ptr<Ast>> body;
	std::vector<FnArg> args;
	FnDecl();

};

class TokenIterator {     
	std::vector<token_t>& tokens;
	unsigned int it;
	public:
	 TokenIterator(std::vector<token_t>& tokens);
     Lexem* pop();
     Lexem* peek();
     Lexem* peek(unsigned int n);
     bool can_iter();

};

class Parser : TokenIterator{
public:
	Parser(std::vector<token_t>& tokens);
	void parse();
private:
	std::unique_ptr<Ast> parse_any();
	std::unique_ptr<Expr> parse_expr();
	 bool is_op(Lexem* op);
	 Op parse_op();
	std::unique_ptr<FnDecl> parse_fn_decl();
	 FnDecl::FnMods parse_fn_mods();
	 std::vector<FnDecl::FnGenArg> parse_fn_gen_args();
	 std::vector<FnDecl::FnArg> parse_fn_args();


	 bool is_typed();
	  bool is_fn_call();
	  bool is_lit();
	  bool is_var();
	  //bool is_lambda_call()
	  //bool is_generic_fn_call()
};