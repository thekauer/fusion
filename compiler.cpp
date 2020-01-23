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
        serror(Error_e::TooFewArgumentsForFs,"expected at least one argument.");
    }

    auto start = std::chrono::high_resolution_clock::now();
    FusionCtx ctx;
    create_fs_std_lib(ctx);

    bool show_llvm=true;
    SourceManager sm;
    //sm.open("main.fs");
    for(int i=1; i<argc; i++) {
        sm.open(argv[i]);
        Lexer l(sm.sources[i-1],ctx);
        l.lex();

        Parser p(l.tokens,ctx);


        auto m = p.parse_fndecl();

        std::cout << "\npretty print:\n";
        m->pretty_print();
        std::cout << "\n\n";
        while(m) {
            m->codegen(ctx);

            m=p.parse_fndecl();
        }
    }




    generate_obj(ctx.mod.get());

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    auto dd =(double)duration/1000;
    system("./compile.sh");
    std::cout << "[" << std::setprecision(4)<< dd << "s]";


    if(show_llvm) {
        llvm::legacy::PassManager PM;
        PM.add(llvm::createPrintModulePass(llvm::outs()));
        PM.run(*ctx.mod);
    }
}

void Compiler::generate_obj(llvm::Module* m,const std::string& filename) {

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
        serror(Error_e::Unk,Error);
    }

    auto CPU="generic";
    auto Features="";
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    llvm::TargetOptions opt;
    auto* TargetMachine = Target->createTargetMachine(targettriple,CPU,Features,opt,RM);
    m->setDataLayout(TargetMachine->createDataLayout());
    std::error_code EC;
    llvm::raw_fd_ostream dest(filename,EC,llvm::sys::fs::OF_None);

    if(EC) {
        std::string s = "could not open file: "+ EC.message();
        serror(Error_e::Unk,s);
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

void Compiler::mod_to_file(llvm::Module* m,const std::string& filename) {
    std::error_code ec;
    llvm::raw_fd_ostream file_stream = llvm::raw_fd_ostream(filename,ec);
    if(ec)std::cout <<ec.message();
    m->print(file_stream,nullptr);
    file_stream.flush();
}


void Compiler::create_fs_std_lib(FusionCtx& ctx) {
    using namespace llvm;
    Type* ity =llvm::IntegerType::getInt32Ty(ctx.ctx);
    llvm::FunctionType* printf_ty = llvm::FunctionType::get(ity, {IntegerType::getInt8PtrTy(ctx.ctx)},true);
    ctx.mod->getOrInsertFunction("printf",printf_ty);



}
