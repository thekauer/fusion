#include "compiler.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Pass.h"
#include "llvm/IR/IRPrintingPasses.h"

#include <chrono>
#include <iomanip>
void Compiler::compile(int argc,char** argv) {
    if(argc<2) {
        std::cout << "expected at least one argument";
        std::exit(1);
    }

    auto start = std::chrono::high_resolution_clock::now();
    FusionCtx ctx;
    create_fs_std_lib(ctx);

    SourceManager sm;
    //sm.open("main.fs");
    for(int i=1;i<argc;i++) {
        sm.open(argv[i]);
        Lexer l(sm.sources[0],ctx);
        l.lex();
        Parser p(l.tokens);
        auto m = p.parse_fndecl();
        if(m) {
            //m->print_name();
        }
        m->codegen(ctx);
    }



    
    generate_obj(ctx.mod.get());
    system("./compile.sh");
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    auto dd =(double)duration/1000;
    std::cout << "[" << std::setprecision(4)<< dd << "s]";

    /*
    llvm::legacy::PassManager PM;
    PM.add(llvm::createPrintModulePass(llvm::outs()));
    PM.run(*ctx.mod);
    */
}

void Compiler::generate_obj(llvm::Module* m) {

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeNativeTarget();

    auto targettriple= llvm::sys::getDefaultTargetTriple();
    m->setTargetTriple(targettriple);
    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(targettriple,Error);
    if(!Target) {
        llvm::errs() << Error;
        std::exit(1);
    }

    auto CPU="generic";
    auto Features="";
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    llvm::TargetOptions opt;
    auto* TargetMachine = Target->createTargetMachine(targettriple,CPU,Features,opt,RM);
    m->setDataLayout(TargetMachine->createDataLayout());
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
        std::exit(1);
    }
    //pass.add(llvm::createPrintModulePass(llvm::outs()));
    pass.run(*m);
    dest.flush();
}

void Compiler::mod_to_file(llvm::Module* m) {
    std::error_code ec;
    llvm::raw_fd_ostream file_stream = llvm::raw_fd_ostream("main.ll",ec);
    if(ec)std::cout <<ec.message();
    m->print(file_stream,nullptr);
    file_stream.flush();
}


void Compiler::create_fs_std_lib(FusionCtx& ctx) {
    using namespace llvm;
    Type* ity =llvm::IntegerType::getInt32Ty(ctx.ctx);
    Type* i8ty = IntegerType::getInt8Ty(ctx.ctx);
    Type* void_ty = Type::getVoidTy(ctx.ctx);
    llvm::FunctionType* printf_ty = llvm::FunctionType::get(ity,{IntegerType::getInt8PtrTy(ctx.ctx)},true);
    ctx.mod->getOrInsertFunction("printf",printf_ty);

    
}