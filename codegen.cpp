#include "parser.h"
#include "llvm/IR/Verifier.h"


llvm::Value* FnCall::codegen(FusionCtx& ctx) {
    auto* fn =ctx.mod->getFunction(name.getName());
    if(!fn) {
        std::string err_msg = "unknown function called " + name.getName();
        error(Error_e::FnNotExsits,err_msg,name.sl);
    }
    std::vector<llvm::Value*> fn_args;
    for(auto&& arg : args) {
        fn_args.push_back(arg->codegen(ctx));
    }
    return ctx.builder.CreateCall(fn,fn_args,"call");
}

llvm::Value* FnProto::codegen(FusionCtx& ctx) {
    std::vector<llvm::Type*> fn_args;
    /*
    for(auto&& arg : args) {
        fn_args.push_back(arg->codegen(ctx)->getType());
    }
    */
    //args and set name for them
    llvm::Type* ret_t=nullptr;
    //if(!ret_t) {
        ret_t =llvm::IntegerType::getInt32Ty(ctx.ctx);
    //}
    //check linkagetype
    llvm::GlobalValue::LinkageTypes lt=llvm::Function::ExternalLinkage;
    llvm::FunctionType* ft = llvm::FunctionType::get(ret_t,fn_args,false);

    llvm::Function* f = llvm::Function::Create(ft,lt,name.getName(),ctx.mod.get());
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

llvm::Value* FnDecl::codegen(FusionCtx& ctx) {
    llvm::Function* p = (llvm::Function*)proto->codegen(ctx);

    auto name = proto->name;
    auto fname = name.getName();
    //auto* fn = mod->getFunction(fname);
    auto* fn = (llvm::Function*)p;
    if(!fn) {
        std::string err_msg = "unknown function called " + fname;
        error(Error_e::FnNotExsits,err_msg,name.sl);
    }
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(ctx.ctx,"entry",fn);
    ctx.builder.SetInsertPoint(bb);
    ctx.named_values.clear();
    for(auto &arg : fn->args()) {
        llvm::AllocaInst* alloca = ctx.builder.CreateAlloca(arg.getType(),nullptr,arg.getName());
        ctx.builder.CreateStore(&arg,alloca);
        ctx.named_values[arg.getName()]=alloca;
    }
    /*
    if(proto->ret->codegen()) {
        //return with returntype
    }else {
    */
        llvm::Constant* def_ret_val = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(ctx.ctx),llvm::APInt(32,0,true));
        ctx.builder.CreateRet(def_ret_val);
    //}

    llvm::verifyFunction(*fn);
    return fn;
}
llvm::Value* ValExpr::codegen(FusionCtx& ctx) {
    return val;
}

llvm::Value* TypeExpr::codegen(FusionCtx& ctx) {
    return reinterpret_cast<llvm::Value*>(type);
}

llvm::Value* BinExpr::codegen(FusionCtx& ctx) {
    return nullptr;
}

llvm::Value* VarExpr::codegen(FusionCtx& ctx) {
    llvm::Value* v = ctx.named_values[name.data()];
    if(!v) {
        //errro unknown value
        return nullptr;
    }
    return ctx.builder.CreateLoad(v,name.data());
}
