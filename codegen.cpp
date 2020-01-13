#include "codegen.h"
#include "parser.h"
/*
CodeGen::CodeGen() {
    mod=std::make_unique<llvm::Module>("test",ctx);
}

llvm::Type* typeof_expr(std::unique_ptr<AstExpr> expr) {
    switch (expr->type)
    {
    case AstType::ValExpr:
        return reinterpret_cast<ValExpr*>(expr.get())->val->getType();
    case AstType::TypeExpr:
        return reinterpret_cast<TypeExpr*>(expr.get())->type;
    case AstType::BinExpr:
        return nullptr;

    default:
        break;
    }
    return nullptr;
}
void CodeGen::gen_decl(std::unique_ptr<FnDecl> expr) {
    llvm::FunctionType* ty;
   if(expr->proto->name=="main") {
    llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(ctx),false);
   }
   llvm::Function* fun = reinterpret_cast<llvm::Function*>(mod->getOrInsertFunction("main",ty).getCallee());
   llvm::BasicBlock* entry = llvm::BasicBlock::Create(ctx,"entry",fun);
   llvm::IRBuilder b(entry);
   


}
void CodeGen::gen_call(std::unique_ptr<FnCall> expr,llvm::BasicBlock* block) {
    llvm::IRBuilder b(block);
    auto* fn =mod->getFunction(expr->name);
}
*/




llvm::Value* FnCall::codegen() {
    auto* fn =mod->getFunction(name);
    if(!fn)//error no function called name exists did you mean...
    return nullptr;
    std::vector<llvm::Value*> fn_args;
    for(auto&& arg : args) {
        fn_args.push_back(arg->codegen());
    }
    return builder.CreateCall(fn,fn_args,"call");
}

llvm::Value* FnProto::codegen() {
    std::vector<llvm::Type*> fn_args;
    for(auto&& arg : args) {
        fn_args.push_back(arg->codegen()->getType());
    }
    //args and set name for them
    llvm::Type* ret_t=nullptr;
    if(!ret_t) {
        ret_t =llvm::IntegerType::getInt32Ty(ctx);
    }
    //check linkagetype
    llvm::GlobalValue::LinkageTypes lt=llvm::Function::ExternalLinkage;

    llvm::FunctionType* ft = llvm::FunctionType::get(ret_t,fn_args,false);
    llvm::Function* f = llvm::Function::Create(ft,lt,name,mod.get());
    //set arg names
    unsigned idx=0;
    for(auto& arg : f->args()) {
        auto n=((ValExpr*)(args[idx++].get()))->val->getName();
        arg.setName(n);
    }

    return f;

}

llvm::Value* FnDecl::codegen() {
    auto* p = proto->codegen();
    auto fn = mod->getFunction(proto->name);
    if(!fn)return nullptr;
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(ctx,"entry",fn);
    builder.SetInsertPoint(bb);
    NamedValues.clear();
    for(auto &arg : fn->args()) {
        llvm::AllocaInst* alloca = builder.CreateAlloca(arg.getType(),nullptr,arg.getName());
        builder.CreateStore(&arg,alloca);
        NamedValues[arg.getName()]=alloca;
    }
    if(proto->ret->codegen()) {
        //return with returntype
    }else {
        llvm::Constant* def_ret_val = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(ctx),llvm::APInt(32,0,true));
        builder.CreateRet(def_ret_val);
    }

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
    llvm::Value* v = NamedValues[name];
    if(!v) {
        //errro unknown value
        return nullptr;
    }
    return builder.CreateLoad(v,name.str().c_str());
}
