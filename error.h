#pragma once
#include "lex.h"
#include <iostream>

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
  UnkEsc,
  CouldNotInferType
};
[[noreturn]] void error(Error_e code, const std::string &msg,
                        const SourceLocation &sl);

[[noreturn]] void serror(Error_e code, const std::string &msg);
