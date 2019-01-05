#include "parser.h"



Parser::Parser(std::vector<token_t>& tokens) : TokenIterator(tokens) {};

void Parser::parse() {
	auto see  = parse_fn_decl();

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

Lexem* TokenIterator::pop() {
	auto ret = peek();
	if(can_iter()) {
		it++;
	}
	return ret;
}

Lexem* TokenIterator::peek() {
	return tokens[it].get();
}
Lexem* TokenIterator::peek(unsigned int n) {
	//TODO: FIX SEGMENTATION FAULT WHEN PEEKING TOO FAR
	if(it<tokens.size()-n) {
		return tokens[it+n].get();
	}
	return tokens[it].get();
}

std::unique_ptr<Ast> Parser::parse_any() {
	return nullptr;
}
bool Parser::is_op(Lexem* op) {
	bool ret=false;
	switch(op->type) {
		case Lp:
		case Rp:
		case Add:
		case Mul:
		case Div:
		case Sub:
		case Mod:
		case Neg:
		case Not:
		case And:
		case Or:
		case Xor:
		case Less:
		case Greater:
		case Eq:
		case Addeq:
		case Subeq:
		case Muleq:
		case Diveq:
		case Modeq:
		case Negeq:
		case Noteq:
		case Andeq:
		case Oreq:
		case Xoreq:
		case Lesseq:
		case Greatereq:
		case Eqeq:
		case AndAnd:
		case OrOr:
		case AddAdd:
		case SubSub:
			ret=true;
			break;
		default:
		break;
	}
	return ret;
}

Op Parser::parse_op() {
	switch(peek()->type) {
		case Eq:
			return Op::Assign;
		case Add:
			return Op::Add;
		case Sub:
			return Op::Sub;
		case Mul:
			return Op::Mul;
		case Div:
			return Op::Div;
		case Not:
			return Op::Not;
		case Mod:
			return Op::Mod;
		case And:
			return Op::BitAnd;
		case Or:
			return Op::BitOr;
		case Xor:
			return Op::BitXor;
		case Less:
			return Op::Less;
		case Greater:
			return Op::Greater;
		case Addeq:
			return Op::Addeq;
		case Subeq:
			return Op::Subeq;
		case Muleq:
			return Op::Muleq;
		case Diveq:
			return Op::Diveq;
		case Modeq:
			return Op::Modeq;
		case Andeq:
			return Op::Andeq;
		case Oreq:
			return Op::Oreq;
		case Xoreq:
			return Op::Xoreq;
		case AndAnd:
			return Op::And;
		case OrOr:
			return Op::Or;
		case Eqeq:
			return Op::Eq;
		case Noteq:
			return Op::Noteq;
		case Lesseq:
			return Op::Lesseq;
		case Greatereq:
			return Op::Greatereq;
		case Dot:
			return Op::Dot;
		case RightArrow:
			return Op::Arrow;
		default:
			throw "error no such Operator";
	}
}
std::unique_ptr<Expr> Parser::parse_expr() {
	//IMPLEMENT ME
	return nullptr;
}

bool Parser::is_typed() {
	return is_lit() || is_var() || is_fn_call();
}
bool Parser::is_lit() {
	switch(peek()->type) {
		case LitChar:
		case LitString:
		case LitBool:
		case LitHex:
		case LitFloat:
		case LitInt:
			return true;
		default:
			return false;
	}
}
bool Parser::is_var() {
	//TODO FIX THIS
	return peek()->type == Id;
}
bool Parser::is_fn_call() {
	return peek()->type == Id && peek(1)->type==Lp; // Greedy
}

FnDecl::FnMods Parser::parse_fn_mods() {
	FnDecl::FnMods mods;
	while(peek()->type == Kw  && static_cast<Key_Word*>(peek())->val != Fn) {
		switch(static_cast<Key_Word*>(pop())->val) {
			case Cte:
				mods.Cte = 1;
				break;
			case Inline:
				mods.Inline =1;
				break;
			case Static:
				mods.Static =1;
				break;
			//case Public:
				//mods.Public =1;
				//break;
			//case Virtual
			//case Override
			case Extern:
				mods.Extern=1;
				break;
			default:
				break;
		}
	} 
	return mods;
}


FnDecl::FnDecl() : Ast(Ast::FnDecl){};


FnDecl::FnGenArg::FnGenArg(u64 name) :name(name){};

std::vector<FnDecl::FnGenArg> Parser::parse_fn_gen_args() {
	std::vector<FnDecl::FnGenArg> args;
	if(peek()->type == Less) {
		pop();
		while(peek()->type == Id) {
			args.push_back(FnDecl::FnGenArg(static_cast<I_d*>(pop())->val));
			if(peek()->type==Comma) {
				pop();
			}
		}
		if(peek()->type == Greater) {
			pop();
		} else {
			//THROW ERROR: generic decl unfinished
		}
	}
	return args;
}
FnDecl::FnArg::FnArg(u64 name) :name(name),type(0){};
FnDecl::FnArg::FnArg(u64 name,u64 type) :name(name),type(type){};

std::vector<FnDecl::FnArg> Parser::parse_fn_args() {
	std::vector<FnDecl::FnArg> ret;
	if(peek()->type ==Lp) {
		pop();
		while(peek()->type != Rp) {
			if(peek()->type == Id) {
				if(peek(1)->type == DoubleDot) {
					if(peek(2)->type == Id) {
						u64 name = static_cast<I_d*>(pop())->val;
						pop();
						u64 type = static_cast<I_d*>(pop())->val;
						ret.push_back(FnDecl::FnArg(name,type));
					} else {
						//THROW ERROR: identifier is not a type
					}
				} else {
					ret.push_back(FnDecl::FnArg(static_cast<I_d*>(pop())->val));
					pop();
				}
			} else {
				break;
			}
		}
	}
	return ret;
}


std::unique_ptr<FnDecl> Parser::parse_fn_decl() {
	int i=0;
	while(peek()->type != N) {
		if(peek(i)->type == Kw && static_cast<Key_Word*>(peek(i))->val == Fn) {
			auto ret = std::unique_ptr<FnDecl>(new FnDecl());
			ret->mods = parse_fn_mods();
			pop();
			if(peek()->type ==Id) {
				ret->name = static_cast<I_d*>(pop())->val;
			} else {
				//THROW ERROR: no function name
			}
			ret->gen_args = parse_fn_gen_args();
			ret->args = parse_fn_args();
			//PARSE RETURN TYPE
			//pop N
			//PARSE BODY
			return ret;

		}
		i++;
	}
	return nullptr;
}


Typed::Typed(type_ type) : Ast(Ast::Typed), type(type){};
Lit::Lit(std::unique_ptr<Lexem> lit) : Typed(Typed::Lit), lit(std::move(lit)){};