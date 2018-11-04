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
	I_d global{ hash("__GLOBAL"),0,0 };
	struct Node {
		I_d& id;
		Type type;
		std::vector<std::unique_ptr<Node>> childs;
		Node(I_d& id, Type type);
	};
	std::unique_ptr<Node> root {std::move(new Node(global,Namespace))};
public:
	Symtable() = default;
	Node* get(std::string path);



};