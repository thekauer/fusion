#include "codegen.h"
#include "parser.h"



llvm::Value* FnCall::codegen() {
    auto* fn =mod->getFunction(name.getName());
    if(!fn) {
        std::string err_msg = "unknown function called " + name.getName();
        error(Error_e::FnNotExsits,err_msg,name.sl);
    }
    std::vector<llvm::Value*> fn_args;
    for(auto&& arg : args) {
        fn_args.push_back(arg->codegen());
    }
    return builder.CreateCall(fn,fn_args,"call");
}

llvm::Value* FnProto::codegen() {
    std::vector<llvm::Type*> fn_args;
    /*
    for(auto&& arg : args) {
        fn_args.push_back(arg->codegen()->getType());
    }
    */
    //args and set name for them
    llvm::Type* ret_t=nullptr;
    //if(!ret_t) {
        ret_t =llvm::IntegerType::getInt32Ty(ctx);
    //}
    //check linkagetype
    llvm::GlobalValue::LinkageTypes lt=llvm::Function::ExternalLinkage;
    llvm::FunctionType* ft = llvm::FunctionType::get(ret_t,fn_args,false);

    llvm::Function* f = llvm::Function::Create(ft,lt,name.getName(),mod.get());
    //set arg names
    /*
    unsigned idx=0;
    for(auto& arg : f->args()) {
        auto n=((ValExpr*)(args[idx++].get()))->val->getName();
        arg.setName(n);
    }
    */
    return f;

}

llvm::Value* FnDecl::codegen() {
    llvm::Function* p = (llvm::Function*)proto->codegen();

    auto name = proto->name;
    auto fname = name.getName();
    //auto* fn = mod->getFunction(fname);
    auto* fn = (llvm::Function*)p;
    if(!fn) {
        std::string err_msg = "unknown function called " + fname;
        error(Error_e::FnNotExsits,err_msg,name.sl);
    }
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(ctx,"entry",fn);
    builder.SetInsertPoint(bb);
    NamedValues.clear();
    for(auto &arg : fn->args()) {
        llvm::AllocaInst* alloca = builder.CreateAlloca(arg.getType(),nullptr,arg.getName());
        builder.CreateStore(&arg,alloca);
        NamedValues[arg.getName()]=alloca;
    }
    /*
    if(proto->ret->codegen()) {
        //return with returntype
    }else {
    */
        llvm::Constant* def_ret_val = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(ctx),llvm::APInt(32,0,true));
        builder.CreateRet(def_ret_val);
    //}

    llvm::verifyFunction(*fn);
    return fn;
}
llvm::Value* ValExpr::codegen() {
    return val;
}

llvm::Value* TypeExpr::codegen() {
    return reinterpret_cast<llvm::Value*>(type);
}

llvm::Value* BinExpr::codegen() {
    return nullptr;
}

llvm::Value* VarExpr::codegen() {
    llvm::Value* v = NamedValues[name.data()];
    if(!v) {
        //errro unknown value
        return nullptr;
    }
    return builder.CreateLoad(v,name.data());
}
