#include "error.h"

Error::Error(std::string file, Iterator it, Type type, std::string msg) : file(file),line(it.line),col(it.col),type(type),msg(msg)
{

}

Error::Error(std::string file, int line, unsigned int col, Type type, std::string msg) :
	file(file),line(line),col(col),type(type),msg(msg)
{

}



void Error::print()
{
	std::string res;
	switch (type) {
	case Err:
		res = "Error";
		break;
	case Warn:
		res = "Warn";
		break;
	case Lint:
		res = "Lint";
		break;
	case Note:
		res="Note";
		break;
	}
	res += " in file \""+file+"\" at <" + std::to_string(line) + ","+std::to_string(col)+">: "+msg;
	std::cout << res << std::endl;

	
}
