#include "parser.h"



Parser::Parser(std::vector<token_t>& tokens) : TokenIterator(tokens) {};

void Parser::parse() {
	
	

}

Ast::Ast(type_ type) : type(type){};

Expr::Expr(type_ type) :Ast(Ast::Expr), type(type){};

UnaryExpr::UnaryExpr(Op op) : Expr(Expr::Unary) , op(op){};
BinaryExpr::BinaryExpr(Op op) : Expr(Expr::Binary), op(op){};
ParenExpr::ParenExpr() : Expr(Expr::Paren){};

TokenIterator::TokenIterator(std::vector<token_t>& tokens) : tokens(tokens),it(0){};	

bool TokenIterator::can_iter() {
	return it<tokens.size()-1;
}

Lexem TokenIterator::pop() {
	auto ret = peek();
	if(can_iter()) {
		it++;
	}
	return ret;
}

Lexem TokenIterator::peek() {
	return *tokens[it];
}
Lexem TokenIterator::peek(unsigned int n) {
	//TODO: FIX SEGMENTATION FAULT WHEN PEEKING TOO FAR
	if(it<tokens.size()-n) {
		return *tokens[it+n];
	}
	return *tokens[it];
}

void Parser::parse_expr() {
	if(is_typed())
}

bool Parser::is_typed() {
	return is_lit() || is_var() || is_fn_call();
}
bool Parser::is_lit() {
	switch(peek().type) {
		case LitChar:
		case LitString:
		case LitBool:
		case LitHex:
		case LiteFloat:
		case LitInt:
			return true;
		default:
			return false;
	}
}
bool Parser::is_var() {
	//TODO FIX THIS
	return peek().type == Id;
}
bool Parser::is_fn_call() {
	return peek().type == Id && peek(1).type==Lp // Greedy
}


Parser::Typed(type_ type) : Ast(Ast::Typed), type(type){};
Parser::Lit(std::unique_ptr<Lexem> lit) : Typed(Typed::Lit), lit(std::move(lit)){};