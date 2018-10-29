#pragma once
#include <iostream>
#include <string>
#include "lexiter.h"

class Error {
public:
	enum Type {
		Err,
		Warn,
		Lint,
		Note
	};
	Error() = delete;
	Error(std::string file, Iterator it, Type type, std::string msg);
	Error(std::string file, int line, unsigned int col, Type type, std::string msg);
	void print();

private:
	unsigned int line, col;
	std::string msg,file;
	Type type;
};