#include <iostream>
#include "parser.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/FileSystem.h"





int main() {
    SourceManager sm;
    sm.open("main.fs");
    /*
    Lexer lex(sm.sources[0]);
    int max=15;
    while(lex.it!=lex.end) 
    std::cout << lex.next().type << " ";
    */
   
    Lexer l(sm.sources[0]);
    l.lex();
    Parser p(l.tokens);
    auto err = SourceLocation(sm.sources[0]);
    err.line=2;
    err.col=11;
    //auto m = p.parse_fndecl();
    auto m = p.parse_fndecl();
    if(m) {
        //std::cout <<"yeey\n";
        m->print_name();
    }
    else std::cout << "ayy";
    
    /*
    */
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeNativeTarget();

    auto targettriple= llvm::sys::getDefaultTargetTriple();
    mod->setTargetTriple(targettriple);
    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(targettriple,Error);
    if(!Target) {
        llvm::errs() << Error;
        return 1;
    }

    auto CPU="generic";
    auto Features="";
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    llvm::TargetOptions opt;
    auto TargetMachine = Target->createTargetMachine(targettriple,CPU,Features,opt,RM);
    mod->setDataLayout(TargetMachine->createDataLayout());
    auto filename = "main.o";
    std::error_code EC;
    llvm::raw_fd_ostream dest(filename,EC,llvm::sys::fs::OF_None);

    if(EC) {
        llvm::errs() << "could not open file: "<< EC.message();
    }
    llvm::legacy::PassManager pass;
    //llvm::FunctionPassManager pass;
    auto FileType = llvm::TargetMachine::CGFT_ObjectFile;

    

    if(TargetMachine->addPassesToEmitFile(pass,dest,nullptr,FileType)) {
        llvm::errs() << "Could not emit to file";
        return 1;
    }
    m->codegen();
    pass.run(*mod);
    dest.flush();

    //error(Error_e::ExpectedToken,"works?",err);
    //auto m = p.parse_fndecl();
    //llvm::Module* mod =  new llvm::Module("tests",ctx);
    //m->print_name();
    //m->proto->print_name();
    //m->body->print_name();
    

    return 0;
}