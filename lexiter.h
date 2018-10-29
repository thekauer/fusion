#pragma once
#include <iostream>
class Iterator {
	typedef unsigned int uint;
	uint i = 0;
	std::string code;
	size_t len;
	char next = '\0', nextnext = '\0';
	bool caniter = true;
	void increment();

public:
	uint col = 0, line = 0;
	Iterator() = delete;
	Iterator(std::string code);
	char pop();
	char peek();
	char peek(unsigned int);
	char peek_next();
	char peek_next_next();
	bool can_iter();

};