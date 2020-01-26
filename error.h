#pragma once
#include "lex.h"
#include <iostream>
struct FSFile;
class SourceLocation;
struct RespawnedCode {
  int line, col;
  std::string code;
  std::string file;
  RespawnedCode(int line, int col, const std::string &code,
                const std::string &file)
      : line(line), col(col), code(code), file(file){};
};

RespawnedCode respawn(const FSFile &file, unsigned int pos);

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
//[[noreturn]] void error(Error_e code, const std::string &msg,const
//SourceLocation &sl);

[[noreturn]] void serror(Error_e code, const std::string &msg);

[[noreturn]] void error(Error_e code, const FSFile &file,
                        const SourceLocation &sl, const std::string &msg);
void note(Error_e code, const FSFile &file, const SourceLocation &sl,
          const std::string &msg);
void warning(Error_e code, const FSFile &file, const SourceLocation &sl,
             const std::string &msg);

class Error {
public:
  static void UnkEsc(const FSFile &file, const SourceLocation &sl,
                     const char ch);
};