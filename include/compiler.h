#pragma once
#include "context.h"
#include "error.h"
#include "lex.h"
#include "parser.h"
#include "source.h"

class Compiler {
public:
  Compiler() = default;
  void compile(int argc, char **argv);
  void test();

private:
  void mod_to_file(llvm::Module *m, const std::string &filename = "main.ll");
  void create_fs_std_lib(FusionCtx &ctx);
  void generate_obj(llvm::Module *m, const std::string &filename = "main.o");
};
