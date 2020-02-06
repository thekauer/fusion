#include "type.h"
#include "error.h"

static IntegralType i8_ty = IntegralType("i8", IntegralType::I8, 1);
static IntegralType i16_ty = IntegralType("i16", IntegralType::I16, 2);
static IntegralType i32_ty = IntegralType("i32", IntegralType::I32, 4);
static IntegralType i64_ty = IntegralType("i64", IntegralType::I64, 8);
static IntegralType u8_ty = IntegralType("u8", IntegralType::U8, 1);
static IntegralType u16_ty = IntegralType("u16", IntegralType::U16, 2);
static IntegralType u32_ty = IntegralType("u32", IntegralType::U32, 4);
static IntegralType u64_ty = IntegralType("u64", IntegralType::U64, 8);

static IntegralType double_ty = IntegralType("double", IntegralType::Double, 8);
Type::Type(const std::string &name, TypeKind tk, By pass, bool mut,
           const unsigned int size, bool optional)
    : name(name), tk(tk), pass(pass), mut(mut), size(size), optional(optional) {
}

Type *Type::toVal() {
  pass = Val;
  return this;
}
Type *Type::toRef() {
  pass = Ref;
  return this;
}

Type *Type::toPtr() {
  pass = Ptr;
  return this;
}
Type *Type::toMut() {
  mut = true;
  return this;
}

Type *Type::toConst() {
  mut = false;
  return this;
}
Type *Type::toOptional() {
  optional = true;
  return this;
}
Type *Type::toNotOption() {
  optional = false;
  return this;
}

unsigned int Type::getSize() { return size; }

const std::string &Type::getName() { return name; }
Type::TypeKind Type::getTypeKind() { return tk; }
Type::By Type::getBy() { return pass; }
Type* Type::setBy(By by) {this->pass=by;return this;}

Type *Type::getI8() { return &i8_ty; }
Type *Type::getI16() { return &i16_ty; }
Type *Type::getI32() { return &i32_ty; }
Type *Type::getI64() { return &i64_ty; }
Type *Type::getISize() { return &i64_ty; }
Type *Type::getU8() { return &u8_ty; }
Type *Type::getU16() { return &u16_ty; }
Type *Type::getU32() { return &u32_ty; }
Type *Type::getU64() { return &u64_ty; }
Type *Type::getUSize() { return &u64_ty; }
Type *Type::getChar() { return &i8_ty; }
Type *Type::getBool() { return &i32_ty; }


bool Type::isSignedIntegerType() {
  return this==getI8() || this == getI16() || this == getI32() || this == getI64();
}
bool Type::isUnsignedIntegerType() {
  return this==getU8() || this == getU16() || this == getU32() || this == getU64();
}

bool Type::isIntegerType() {
  return isSignedIntegerType() || isUnsignedIntegerType();
}

Type *Type::getDouble() {return &double_ty;}

static int struct_size(const std::vector<StructType::TypedValue> &members) {
  int s = 0;
  for (const auto &v : members) {
    s += v.ty->getSize();
  }
  return s;
}
static int tuple_size(std::vector<Type *> members) {
  int s = 0;
  for (const auto &m : members) {
    s += m->getSize();
  }
  return s;
}
IntegralType::IntegralType(const std::string &name, Ty ty,
                           const unsigned int size)
    : Type(name, Type::Integral, Type::Val, false, size, false), ty(ty) {}

StructType::StructType(const std::string &name,
                       const std::vector<TypedValue> &members)
    : Type(name, Type::Struct, Type::Val, false, struct_size(members), false),
      members(members){};

TupleType::TupleType(const std::string &name,
                     const std::vector<Type *> &members)
    : Type(name, Type::Tuple, Type::Val, false, tuple_size(members), false) {}

llvm::Type *IntegralType::codegen(FusionCtx &ctx) {
  llvm::Type *ret;
  switch (ty) {
  case Bool:
  case Char:
  case U8:
  case I8:
    ret = ctx.getI8();
    break;
  case U16:
  case I16:
    ret = ctx.getI16();
    break;
  case U32:
  case I32:
    ret = ctx.getI32();
    break;
  case U64:
  case I64:
    ret = ctx.getI64();
    break;
  default:
    return nullptr;
  }
  if (pass == Ptr) {
    ret->getPointerTo();
  }
  return ret;
}

llvm::Type *StructType::codegen(FusionCtx &ctx) {
  serror(Error_e::Unk, "Struct type needs to be implemented");
}
llvm::Type *TupleType::codegen(FusionCtx &ctx) {
  serror(Error_e::Unk, "Tuple type needs to be implemented");
}
