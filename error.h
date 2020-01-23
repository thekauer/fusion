#pragma once
#include <iostream>
#include "lex.h"

class SourceLocation;
enum class Error_e : int {
    Unk,
    TabsnSpaces,
    ExpectedToken,
    EmptyFnBody,
    FnNotExsits,
    FileNExists,
    MustbeFsFile,
    TooFewArgumentsForFs,
    UnkEsc
};
[[noreturn]]
void error(Error_e code,const std::string& msg,const SourceLocation& sl);

[[noreturn]]
void serror(Error_e code,const std::string& msg);
