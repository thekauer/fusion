#include "parser.h"
#include "type.h"
#include "llvm/IR/Verifier.h"
#include "error.h"
static bool type_check(llvm::Type *lhs, llvm::Type *rhs) {
  if (rhs->isPointerTy()) {
    return rhs->isPointerTy();
  }
  if (lhs->isIntegerTy()) {
    return rhs->isIntegerTy();
  }
  if (lhs->isFloatingPointTy()) {
    return rhs->isFloatingPointTy();
  }
  return false;
}
static llvm::Value *alloc(FusionCtx &ctx, llvm::Type *ty, std::string name) {
  llvm::AllocaInst *val = ctx.builder.CreateAlloca(ty, nullptr, name);
  ctx.named_values[name] = val;
  return val;
}

llvm::Value *FnCall::codegen(FusionCtx &ctx) const {
  auto *fn = ctx.mod->getFunction(name);
  if (!fn) {
    std::string err_msg = "unknown function called " + name;
    Error::ImplementMe(err_msg);
    return nullptr;
  }
  std::vector<llvm::Value *> fn_args;
  for (auto &&arg : args) {
      if (arg) {
          fn_args.push_back(arg->codegen(ctx));
      }
  }
  return ctx.builder.CreateCall(fn, fn_args, "call");
}

llvm::Value *FnProto::codegen(FusionCtx &ctx) const {
  std::vector<llvm::Type *> fn_args;
  for (auto &&arg : args) {
    if (arg->type == AstType::VarDeclExpr) {
      auto vd = reinterpret_cast<VarDeclExpr *>(arg.get());
      fn_args.push_back(vd->ty.get_type_ptr()->codegen(ctx));
    }
  }


  llvm::Type *ret_t = ctx.getI32();
  llvm::GlobalValue::LinkageTypes lt = llvm::Function::ExternalLinkage;
  llvm::FunctionType *ft = llvm::FunctionType::get(ret_t, fn_args, false);

  llvm::Function *f = llvm::Function::Create(ft, lt, name, ctx.mod.get());
  // set arg names
  unsigned idx = 0;
  for (auto &arg : f->args()) {
    auto n = ((VarDeclExpr *)(args[idx++].get()))->name;
    arg.setName(n);
    ;
  }

  return f;
}

llvm::Value *FnDecl::codegen(FusionCtx &ctx) const {
  auto *p = (llvm::Function *)proto->codegen(ctx);

  auto fname = proto->name;
  auto *fn = (llvm::Function *)p;
  if (mods & FnModifiers::Extern) {
    ctx.mod->getOrInsertFunction(fname, p->getFunctionType());
    return fn;
  }
  if (!fn) {
    std::string err_msg = "unknown function called " + fname;
    Error::ImplementMe(err_msg);
    return nullptr;
  }
  llvm::BasicBlock *bb = llvm::BasicBlock::Create(ctx.ctx, "entry", fn);
  ctx.builder.SetInsertPoint(bb);
  ctx.named_values.clear();
  for (auto &arg : fn->args()) {
    llvm::AllocaInst *alloca =
        ctx.builder.CreateAlloca(arg.getType(), nullptr, arg.getName());
    ctx.builder.CreateStore(&arg, alloca);
    ctx.named_values[arg.getName().data()] = alloca; // probably buggy
  }

  body->codegen(ctx);


  llvm::verifyFunction(*fn);
  return fn;
}
llvm::Value *ValExpr::codegen(FusionCtx &ctx) const {
  auto const &type = val.ty.get_type();
  if (type.get_typekind() != Type::Integral) {
    return nullptr;
  }
  auto const &it = static_cast<const IntegralType &>(type);
  switch (it.ty) {
  case IntegralType::String: {
    llvm::Type *i8ty = llvm::IntegerType::getInt8Ty(ctx.ctx);
    llvm::ArrayType *sty = llvm::ArrayType::get(i8ty, val.as.string.size() + 1);
    std::vector<llvm::Constant *> vals;
    llvm::GlobalVariable *gstr = new llvm::GlobalVariable(
        *ctx.mod, sty, true, llvm::GlobalValue::PrivateLinkage, 0, "str");
    gstr->setAlignment(1);
    llvm::Constant *cstr =
        llvm::ConstantDataArray::getString(ctx.ctx, val.as.string.data(), true);
    gstr->setInitializer(cstr);
    return (llvm::Constant *)llvm::ConstantExpr::getBitCast(
        gstr, ctx.getI8()->getPointerTo());
  }
  case IntegralType::I32: {
    return llvm::ConstantInt::get(ctx.getI32(),
                                  llvm::APInt(32, val.as.i32, true));
  }
  case IntegralType::F32: {
    return llvm::ConstantFP::get(ctx.ctx, llvm::APFloat(val.as.f32));
  }
  case IntegralType::Bool: {
      if (val.as.b) {
          return llvm::ConstantInt::getTrue(ctx.ctx);
      }
      else {
          return llvm::ConstantInt::getFalse(ctx.ctx);
      }

  }
  default:
    Error::ImplementMe("Implement codegeneration for this type in ValExpr::codegen");
    return nullptr;
  }

  return nullptr;
}

llvm::Value *TypeExpr::codegen(FusionCtx &ctx) const {
  return reinterpret_cast<llvm::Value *>(ty.get_type_ptr()->codegen(ctx));
}

llvm::Value *BinExpr::codegen(FusionCtx &ctx) const {
  switch (op) {
  case Token::Eq: {
    auto *vlhs = lhs->codegen(ctx);
    auto *vrhs = rhs->codegen(ctx);
    if (!vrhs) {
        Error::ImplementMe("No value");
        return nullptr;
    }
    if (vlhs == (llvm::Value *)~0) { // left hand side is an infered type decl
      auto *ty = vrhs->getType();
      if (lhs->type != AstType::VarExpr) {
          Error::ImplementMe("expected left-hand side to be varexpr");
          return nullptr;
      }
      std::string name = reinterpret_cast<VarExpr *>(lhs.get())->name;
      auto *var = alloc(ctx, ty, name);
      return ctx.builder.CreateStore(vrhs, var);
    }

    if (!type_check(vrhs->getType(), vlhs->getType())) {
        Error::ImplementMe("types don't match");
        return nullptr;
    }

    llvm::Value *var = ctx.named_values[vlhs->getName()];
    return ctx.builder.CreateStore(vrhs, var);
    return vrhs;
  }
  case Token::Add: {
    // fix
    auto ity = llvm::IntegerType::getInt32Ty(ctx.ctx);
    auto callee = ctx.mod->getOrInsertFunction("addi32i32", ity, ity, ity);
    return ctx.builder.CreateCall(callee,
                                  {lhs->codegen(ctx), rhs->codegen(ctx)});
    return nullptr;
  }
  case Token::Mul: {
    auto ity = llvm::IntegerType::getInt32Ty(ctx.ctx);
    auto callee = ctx.mod->getOrInsertFunction("muli32i32", ity, ity, ity);
    return ctx.builder.CreateCall(callee,
                                  {lhs->codegen(ctx), rhs->codegen(ctx)});
    return nullptr;
  }
  default:
    Error::ImplementMe("Codegen: Unimplemented operator!");
    return nullptr;
  }
  return nullptr;
}

llvm::Value *VarExpr::codegen(FusionCtx &ctx) const {
  // if the variable was already declared load it
  // else if it doesn't exists then return fullptr
  if (ctx.named_values.find(name) != ctx.named_values.end()) {
    llvm::Value *v = ctx.named_values[name.data()];
    if (!v) {
      // errro unknown value
      return nullptr;
    }
    return ctx.builder.CreateLoad(v, name.data());
  } else {
    return (llvm::Value *)~0;
  }
  return 0;
}

llvm::Value *VarDeclExpr::codegen(FusionCtx &ctx) const {
  if (!ty.get_type_ptr()) {
    Error::ImplementMe("could not infer type of expression");
    return nullptr;
  }
  if (ctx.named_values.find(name) != ctx.named_values.end()) {
    return ctx.named_values[name.data()];
  }
  llvm::AllocaInst *val =
      ctx.builder.CreateAlloca(ty.get_type_ptr()->codegen(ctx), nullptr, name);
  ctx.named_values[name] = val;
  return val;
}

llvm::Value *RangeExpr::codegen(FusionCtx &ctx) const {
  Error::ImplementMe("implement RangeExpr");
  return nullptr; // implement me
}

llvm::Value* Body::codegen(FusionCtx& ctx) const {
    for (auto const& line : body) {
        line->codegen(ctx);
    }
    return nullptr;
}

llvm::Value *IfStmt::codegen(FusionCtx &ctx) const { 
    auto* condv = condition->codegen(ctx);
    if (!condv) {
        return nullptr;
    }
    condv =ctx.builder.CreateICmpEQ(condv, llvm::ConstantInt::getTrue(ctx.ctx), "ifcond");
    llvm::Function* func = ctx.builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* thenbb = llvm::BasicBlock::Create(ctx.ctx, "then");
    llvm::BasicBlock* elsebb = llvm::BasicBlock::Create(ctx.ctx, "else");
    llvm::BasicBlock* mergebb = llvm::BasicBlock::Create(ctx.ctx, "merge");

    ctx.builder.CreateCondBr(condv, thenbb, elsebb);

    ctx.builder.SetInsertPoint(thenbb);
    auto* thenv = body->codegen(ctx);
    if (!thenv)
        return nullptr;
    ctx.builder.CreateBr(mergebb);
    thenbb = ctx.builder.GetInsertBlock();

    func->getBasicBlockList().push_back(elsebb);
    ctx.builder.SetInsertPoint(elsebb);

    auto* elsev = else_body->codegen(ctx);
    if (!elsev)
        return nullptr;

    ctx.builder.CreateBr(mergebb);
    elsebb = ctx.builder.GetInsertBlock();

    func->getBasicBlockList().push_back(mergebb);
    ctx.builder.SetInsertPoint(mergebb);
    auto* pn = ctx.builder.CreatePHI(llvm::Type::getDoubleTy(ctx.ctx), 2, "iftmp");
    pn->addIncoming(thenv, thenbb);
    pn->addIncoming(elsev, elsebb);

    return pn; 
}

llvm::Value *ImportExpr::codegen(FusionCtx &ctx) const {
  // compile module
  Error::ImplementMe("ImportExpr::codegen");
  return nullptr;
}

llvm::Value* ReturnStmt::codegen(FusionCtx& ctx) const
{
    auto *vexpr = expr->codegen(ctx);
    if (vexpr) {
        return ctx.builder.CreateRet(vexpr);
    }
    return nullptr;
}
llvm::Value* ClassStmt::codegen(FusionCtx& ctx) const {
    body->codegen(ctx);
    return reinterpret_cast<llvm::Value*>(ty.get_type_ptr()->codegen(ctx));
}