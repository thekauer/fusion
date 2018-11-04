#pragma once
#include "lex.h"

class Symtable {
public:
	enum Type {//Decls
		Var,
		Class,
		Struct,
		Enum,
		Fn,
		Namespace,
		Mod
	};



private:
	class Node {
		I_d& id;
		Type type;
		std::vector<std::unique_ptr<Node>> childs;
	};
};