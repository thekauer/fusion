#include "type.h"

static const IntegralType i8_ty = IntegralType(IntegralType::I8, 1);
static const IntegralType i16_ty = IntegralType(IntegralType::I16, 2);
static const IntegralType i32_ty = IntegralType(IntegralType::I32, 4);
static const IntegralType i64_ty = IntegralType(IntegralType::I64, 8);

static const IntegralType u8_ty = IntegralType(IntegralType::U8, 1);
static const IntegralType u16_ty = IntegralType(IntegralType::U16, 2);
static const IntegralType u32_ty = IntegralType(IntegralType::U32, 4);
static const IntegralType u64_ty = IntegralType(IntegralType::U64, 8);

Type::Type(TypeKind tk,By pass, bool mut, const unsigned int size, bool optional)
    : tk(tk),pass(pass), mut(mut), size(size), optional(optional) {}

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

const Type *Type::getI8() { return &i8_ty; }
const Type *Type::getI16() { return &i16_ty; }
const Type *Type::getI32() { return &i32_ty; }

const Type *Type::getI64() { return &i64_ty; }
const Type *Type::getISize() { return &i64_ty; }

const Type *Type::getU8() { return &u8_ty; }
const Type *Type::getU16() { return &u16_ty; }
const Type *Type::getU32() { return &u32_ty; }
const Type *Type::getU64() { return &u64_ty; }
const Type *Type::getUSize() { return &u64_ty; }

const Type *Type::getChar() { return &i8_ty; }
const Type *Type::getBool() { return &i32_ty; }
static int struct_size(const std::vector<StructType::TypedValue> &members) {
  int s = 0;
  for (const auto &v : members) {
    s += v.ty->size;
  }
  return s;
}
static int tuple_size(std::vector<Type *> members) {
  int s = 0;
  for (const auto &m : members) {
    s += m->size;
  }
  return s;
}
IntegralType::IntegralType(Ty ty, const unsigned int size)
    : Type(Type::Integral,Type::Val, false, size, false), ty(ty) {}

StructType::StructType(const std::vector<TypedValue> &members)
    : Type(Type::Struct,Type::Val, false, struct_size(members), false), members(members){};

TupleType::TupleType(const std::vector<Type *> &members)
    : Type(Type::Tuple,Type::Val, false, tuple_size(members), false) {}


