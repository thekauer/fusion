#pragma once
#include <iostream>
#include "lex.h"

enum class Error_e : int {
    TabsnSpaces,
    ExpectedToken
};
[[noreturn]]
void error(Error_e code,const std::string& msg,const SourceLocation& sl);
