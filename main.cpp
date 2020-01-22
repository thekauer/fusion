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
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Pass.h"
#include "llvm/IR/IRPrintingPasses.h"


int geno(llvm::Module* m) {

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
        return 1;
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
        return 1;
    }
    pass.add(llvm::createPrintModulePass(llvm::outs()));
    pass.run(*m);
    dest.flush();
}

std::unique_ptr<llvm::Module> gen_test(FusionCtx& ctx) {
    using namespace llvm;
    auto m = std::make_unique<Module>("test",ctx.ctx);

    Type* ity =llvm::IntegerType::getInt32Ty(ctx.ctx);
    Type* i8ty = IntegerType::getInt8Ty(ctx.ctx);
    llvm::FunctionType* printf_ty = llvm::FunctionType::get(ity, {IntegerType::getInt8PtrTy(ctx.ctx)},true);
    m->getOrInsertFunction("printf",printf_ty);

    FunctionType* main_ty = FunctionType::get(ity, {},false);
    Function* main_fn = Function::Create(main_ty,Function::ExternalLinkage,"main",m.get());
    StringRef main_name = "main";
    //auto main_fn = m->getFunction(main_name);
    BasicBlock* bb = BasicBlock::Create(ctx.ctx,"entry",main_fn);
    ctx.builder.SetInsertPoint(bb);
    Constant* c1 = llvm::ConstantInt::get(ity,APInt(32,2,true));
    Constant* c2 = llvm::ConstantInt::get(ity,APInt(32,3,true));
    Value *res = ctx.builder.CreateAdd(c1,c2,"result");
    Constant* shw =ctx.builder.CreateGlobalStringPtr("hello world\n");
    ctx.builder.CreateCall(m->getFunction("printf"), {shw});
    ctx.builder.CreateRet(res);
    std::string err;
    auto os =raw_string_ostream(err);
    if(verifyFunction(*main_fn),&os) {
        std::cout <<"nem joh\n";
        std::cout <<err <<"<-- hiba\n";

    }

    return m;

}


void mod_to_file(llvm::Module* m) {
    std::error_code ec;
    llvm::raw_fd_ostream file_stream = llvm::raw_fd_ostream("main.ll",ec);
    if(ec)std::cout <<ec.message();
    m->print(file_stream,nullptr);
    file_stream.flush();
}

void create_fs_std_lib(FusionCtx& ctx) {
    using namespace llvm;
    Type* ity =llvm::IntegerType::getInt32Ty(ctx.ctx);
    Type* i8ty = IntegerType::getInt8Ty(ctx.ctx);
    Type* void_ty = Type::getVoidTy(ctx.ctx);
    llvm::FunctionType* printf_ty = llvm::FunctionType::get(ity, {IntegerType::getInt8PtrTy(ctx.ctx)},true);
    ctx.mod->getOrInsertFunction("printf",printf_ty);


}


int main() {
    FusionCtx ctx;

    SourceManager sm;
    sm.open("main.fs");

    Lexer l(sm.sources[0],ctx);
    l.lex();
    Parser p(l.tokens);
    create_fs_std_lib(ctx);
    //auto m = p.parse_fndecl();

    auto m = p.parse_fndecl();
    if(m) {
        m->print_name();
    }
    llvm::Function* f =(llvm::Function*)m->codegen(ctx);


    //geno(std::move(mod));

    auto beg = ctx.mod->getFunctionList().begin();
    for(; beg!=ctx.mod->getFunctionList().end(); beg++) {
        std::cout << beg->getName().begin() << "\n";
    }
    llvm::legacy::PassManager PM;
    PM.add(llvm::createPrintModulePass(llvm::outs()));
    PM.run(*ctx.mod);
    mod_to_file(ctx.mod.get());


    return 0;
}
