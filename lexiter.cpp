#include "lexiter.h"


void Iterator::increment()
{
	if (code[i] == '\n') { line++; col = 0; }
	else col++;
	i++;
}

Iterator::Iterator(std::string code) : code(code), len(code.size())
{
	switch (len) {
	case 0:
		break;
	case 1:
		nextnext = '\0'; next = '\0';
		break;
	case 2:
		next = code[i + 1]; nextnext = '\0';
		break;

	default:
		next = code[i + 1];
		nextnext = code[i + 2];
	}
}

char Iterator::pop()
{
	char res = '\0';
	if (i < len -1) {
		res = code[i];
		increment();
		return res;
	}
	
		if (i == len - 1) {
			next = nextnext = '\0';
			res = code[i];

			if (code[i] == '\n') { line++; col = 0; }
			else col++;
			//NO INCREMENTING
			return res;
		}

	return res;
}


void Iterator::pop(unsigned int n) {
	for(unsigned int i=0;i<n;i++) {
		pop();
	}
}

char Iterator::peek()
{
	if (i < len - 1) {
		return code[i];
	}
	return '\0';
}

char Iterator::peek(unsigned int n)
{
	if (i+n-1 < len - 1) {
		return code[i + n];
	}
	return '\0';
}


bool Iterator::can_iter()
{
	return (i<len-1);
}
