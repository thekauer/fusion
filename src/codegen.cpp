#include "error.h"
#include "parser.h"
#include "type.h"
#include "llvm/IR/Verifier.h"
static bool codegen_type_check(llvm::Type *lhs, llvm::Type *rhs) {
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
    if (arg->ast_type == AstType::VarDeclExpr) {
      auto vd = reinterpret_cast<VarDeclExpr *>(arg.get());
      fn_args.push_back(vd->type.get_type_ptr()->codegen(ctx));
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
  auto const &type = val.type.get_type();
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
    } else {
      return llvm::ConstantInt::getFalse(ctx.ctx);
    }
  }
  default:
    Error::ImplementMe(
        "Implement codegeneration for this type in ValExpr::codegen");
    return nullptr;
  }

  return nullptr;
}

llvm::Value *TypeExpr::codegen(FusionCtx &ctx) const {
  return reinterpret_cast<llvm::Value *>(type.get_type_ptr()->codegen(ctx));
}
llvm::Value* codegen_eq(FusionCtx& ctx,AstExpr* lhs,AstExpr* rhs) {
  auto *vlhs = lhs->codegen(ctx);
  auto *vrhs = rhs->codegen(ctx);
  if (!vrhs) {
    Error::ImplementMe("No value");
    return nullptr;
  }
  if (vlhs == (llvm::Value *)~0) { // left hand side is an infered type decl
    auto *ty = vrhs->getType();
    if (lhs->ast_type != AstType::VarExpr) {
      Error::ImplementMe("expected left-hand side to be varexpr");
      return nullptr;
    }
    std::string name = reinterpret_cast<VarExpr *>(lhs)->name;
    auto *var = alloc(ctx, ty, name);
    return ctx.builder.CreateStore(vrhs, var);
  }

  if (!codegen_type_check(vrhs->getType(), vlhs->getType())) {
    Error::ImplementMe("types don't match");
    return nullptr;
  }

  llvm::Value *var = ctx.named_values[vlhs->getName()];
  return ctx.builder.CreateStore(vrhs, var);
  return vrhs;
}

llvm::Value *codegen_add(FusionCtx &ctx, AstExpr *lhs, AstExpr *rhs) {
  bool lhs_isintegral =
      lhs->cast<Expr>()->type.get_type_ptr()->get_typekind() == Type::Integral;
  bool rhs_isintegral =
      rhs->cast<Expr>()->type.get_type_ptr()->get_typekind() == Type::Integral;

  if (lhs_isintegral && rhs_isintegral) {
    return ctx.builder.CreateAdd(lhs->codegen(ctx), rhs->codegen(ctx));
  } else {
    Error::ImplementMe("implement op+ for non integarl types");
  }
  return nullptr;
}
llvm::Value *codegen_sub(FusionCtx &ctx, AstExpr *lhs, AstExpr *rhs) {
  bool lhs_isintegral =
      lhs->cast<Expr>()->type.get_type_ptr()->get_typekind() == Type::Integral;
  bool rhs_isintegral =
      rhs->cast<Expr>()->type.get_type_ptr()->get_typekind() == Type::Integral;

  if (lhs_isintegral && rhs_isintegral) {
    return ctx.builder.CreateSub(lhs->codegen(ctx), rhs->codegen(ctx));
  } else {
    Error::ImplementMe("implement op- for non integarl types");
  }
  return nullptr;
}
llvm::Value *codegen_mul(FusionCtx &ctx, AstExpr *lhs, AstExpr *rhs) {
  bool lhs_isintegral =
      lhs->cast<Expr>()->type.get_type_ptr()->get_typekind() == Type::Integral;
  bool rhs_isintegral =
      rhs->cast<Expr>()->type.get_type_ptr()->get_typekind() == Type::Integral;

  if (lhs_isintegral && rhs_isintegral) {
    auto lhs_integraltype =
        ((IntegralType *)(lhs->cast<Expr>()->type.get_type_ptr()))->ty;
    auto rhs_integraltype =
        ((IntegralType *)(rhs->cast<Expr>()->type.get_type_ptr()))->ty;
    if (lhs_integraltype==IntegralType::F32 || lhs_integraltype == IntegralType::F64 ||
        rhs_integraltype == IntegralType::F32 ||
        rhs_integraltype == IntegralType::F64) {
      return ctx.builder.CreateFMul(lhs->codegen(ctx), rhs->codegen(ctx));
    }
    if (lhs_integraltype == IntegralType::I32 &&
        rhs_integraltype == IntegralType::I32) {
      return ctx.builder.CreateMul(lhs->codegen(ctx), rhs->codegen(ctx));
    }
    return nullptr;
  } else {
    Error::ImplementMe("implement op* for non integarl types");
  }
  return nullptr;
}
llvm::Value *codegen_div(FusionCtx &ctx, AstExpr *lhs, AstExpr *rhs) {
  bool lhs_isintegral =
      lhs->cast<Expr>()->type.get_type_ptr()->get_typekind() == Type::Integral;
  bool rhs_isintegral =
      rhs->cast<Expr>()->type.get_type_ptr()->get_typekind() == Type::Integral;

  if (lhs_isintegral && rhs_isintegral) {
    auto lhs_integraltype =
        ((IntegralType *)(lhs->cast<Expr>()->type.get_type_ptr()))->ty;
    if (lhs_integraltype == IntegralType::F32 ||
        lhs_integraltype == IntegralType::F64) {
        return ctx.builder.CreateFDiv(lhs->codegen(ctx), rhs->codegen(ctx));
    } else {
        //error nonfloatdivrhs
        //note cast it to f32
      return nullptr;
    }
  } else {
    Error::ImplementMe("implement op/ for non integarl types");
  }
  return nullptr;
}
llvm::Value *codegen_mod(FusionCtx &ctx, AstExpr *lhs, AstExpr *rhs) {
  bool lhs_isintegral =
      lhs->cast<Expr>()->type.get_type_ptr()->get_typekind() == Type::Integral;
  bool rhs_isintegral =
      rhs->cast<Expr>()->type.get_type_ptr()->get_typekind() == Type::Integral;

  if (lhs_isintegral && rhs_isintegral) {
      return ctx.builder.CreateSDiv(lhs->codegen(ctx), rhs->codegen(ctx));

  } else {
    Error::ImplementMe("implement op/ for non integarl types");
  }
  return nullptr;
}


llvm::Value *BinExpr::codegen(FusionCtx &ctx) const {
  
  switch (op) {
  case Token::Eq: {
    return codegen_eq(ctx, lhs.get(), rhs.get());
  }
  case Token::Add: {
    return codegen_add(ctx, lhs.get(), rhs.get());
  }
  case Token::Sub: {
    return codegen_sub(ctx, lhs.get(), rhs.get());
  }
  case Token::Mul: {
    return codegen_mul(ctx, lhs.get(), rhs.get());
  }
  case Token::Div: {
    return codegen_div(ctx, lhs.get(), rhs.get());
  }
  case Token::Mod: {
    return codegen_mod(ctx, lhs.get(), rhs.get());
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
  if (!type.get_type_ptr()) {
    Error::ImplementMe("could not infer type of expression");
    return nullptr;
  }
  if (ctx.named_values.find(name) != ctx.named_values.end()) {
    return ctx.named_values[name.data()];
  }
  if (!type.get_type_ptr()) {
    return nullptr;
  }
  auto *typ = type.get_type_ptr();
  auto *tyc = typ->codegen(ctx);
  if (tyc) {

    llvm::AllocaInst *val = ctx.builder.CreateAlloca(tyc, nullptr, name);
    ctx.named_values[name] = val;
    return val;
  }
  return nullptr;
}

llvm::Value *RangeExpr::codegen(FusionCtx &ctx) const {
  Error::ImplementMe("implement RangeExpr");
  return nullptr; // implement me
}

llvm::Value *Body::codegen(FusionCtx &ctx) const {
  llvm::Value *last = nullptr;
  for (auto const &line : body) {
    last = line->codegen(ctx);
  }
  return last;
}

llvm::Value *IfStmt::codegen(FusionCtx &ctx) const {
  auto *condv = condition->codegen(ctx);
  if (!condv) {
    return nullptr;
  }
  condv = ctx.builder.CreateICmpEQ(condv, llvm::ConstantInt::getTrue(ctx.ctx),
                                   "ifcond");
  llvm::Function *func = ctx.builder.GetInsertBlock()->getParent();

  llvm::BasicBlock *thenbb = llvm::BasicBlock::Create(ctx.ctx, "then",func);
  llvm::BasicBlock *elsebb = llvm::BasicBlock::Create(ctx.ctx, "else",func);
  llvm::BasicBlock *mergebb = llvm::BasicBlock::Create(ctx.ctx, "merge",func);
  llvm::Value *elsev = nullptr;
  if (else_body) {
    ctx.builder.CreateCondBr(condv, thenbb, elsebb);
  } else {
    ctx.builder.CreateCondBr(condv, thenbb, mergebb);
  }
  ctx.builder.SetInsertPoint(thenbb);
  auto *thenv = body->codegen(ctx);
  if (!thenv)
    return nullptr;
  ctx.builder.CreateBr(mergebb);
  thenbb = ctx.builder.GetInsertBlock();
  if (else_body) {

    func->getBasicBlockList().push_back(elsebb);
    ctx.builder.SetInsertPoint(elsebb);

    elsev = else_body->codegen(ctx);
    if (!elsev)
      return nullptr;

    ctx.builder.CreateBr(mergebb);
    elsebb = ctx.builder.GetInsertBlock();
  }
  func->getBasicBlockList().push_back(mergebb);
  ctx.builder.SetInsertPoint(mergebb);
  /*
  auto *pn =
      ctx.builder.CreatePHI(llvm::Type::getInt32Ty(ctx.ctx), 2, "iftmp");
  pn->addIncoming(thenv, thenbb);
  if (else_body) {
    pn->addIncoming(elsev, elsebb);
  }

  return pn;*/
  return mergebb;
}

llvm::Value *ImportExpr::codegen(FusionCtx &ctx) const {
  // compile module
  Error::ImplementMe("ImportExpr::codegen");
  return nullptr;
}

llvm::Value *ReturnStmt::codegen(FusionCtx &ctx) const {
  auto *vexpr = expr->codegen(ctx);
  if (vexpr) {
    return ctx.builder.CreateRet(vexpr);
  }
  return nullptr;
}
llvm::Value *ClassStmt::codegen(FusionCtx &ctx) const {
  std::vector<QualType> tys;
  for (const auto &line : body->body) {
    if (line->ast_type != AstType::VarDeclExpr) {
      tys.push_back(line->cast<VarDeclExpr>()->type);
    }
  }
  auto *tyv = StructType(name->cast<VarExpr>()->name, std::move(tys)).codegen(ctx);
  return reinterpret_cast<llvm::Value *>(tyv);
}