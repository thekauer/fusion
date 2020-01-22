#pragma once
#include "lex.h"
#include "parser.h"
#include "source.h"
#include "context.h"


class Compiler {
public:
    Compiler()=default;
    void compile(int argc,char** argv);
private:
    
    void mod_to_file(llvm::Module* m);
    void create_fs_std_lib(FusionCtx& ctx);
    void generate_obj(llvm::Module* m);
};