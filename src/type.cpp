#include "type.h"
#include "error.h"

Type::Type(const std::string_view name, TypeKind tk, const unsigned int size) : name(name), tk(tk), size(size) {}


unsigned int Type::get_size() {
    return size;
}

std::string_view Type::get_name() {
    return name;
}

Type::TypeKind Type::get_typekind() {
    return tk;
}

std::unique_ptr<Type> Type::get_i8() {
    return std::make_unique<IntegralType>("i8", IntegralType::I8, 1);
}

std::unique_ptr<Type> Type::get_i16() {
    return std::make_unique<IntegralType>("i16", IntegralType::I16, 2);
}

std::unique_ptr<Type> Type::get_i32() {
    return std::make_unique<IntegralType>("i32", IntegralType::I32, 4);
}

std::unique_ptr<Type> Type::get_i64() {
    return std::make_unique<IntegralType>("i64", IntegralType::I64, 8);
}

std::unique_ptr<Type> Type::get_isize() {
    return std::make_unique<IntegralType>("isize", IntegralType::ISize, 4);
}
std::unique_ptr<Type> Type::get_u8() {
    return std::make_unique<IntegralType>("u8", IntegralType::U8, 1);
}

std::unique_ptr<Type> Type::get_u16() {
    return std::make_unique<IntegralType>("u16", IntegralType::U16, 2);
}

std::unique_ptr<Type> Type::get_u32() {
    return std::make_unique<IntegralType>("u32", IntegralType::U32, 4);
}

std::unique_ptr<Type> Type::get_u64() {
    return std::make_unique<IntegralType>("u64", IntegralType::U64, 8);
}

std::unique_ptr<Type> Type::get_usize() {
    return std::make_unique<IntegralType>("usize", IntegralType::USize, 4);
}

std::unique_ptr<Type> Type::get_char() {
    return std::make_unique<IntegralType>("i8", IntegralType::I8, 1);
}

std::unique_ptr<Type> Type::get_bool() {
    return std::make_unique<IntegralType>("i8", IntegralType::I8, 1);
}

std::unique_ptr<Type> Type::get_f32() {
    return std::make_unique<IntegralType>("f32", IntegralType::F32, 4);
}
std::unique_ptr<Type> Type::get_f64() {
    return std::make_unique<Type>("f64", IntegralType::F64, 8);
}





IntegralType::IntegralType(const std::string_view name, Ty ty, const unsigned int size) : ty(ty), Type(name, Integral, size) {}

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
  }/*
  if (mods &= Ptr) {
    ret->getPointerTo();
  }*/
  return ret;
}

QualType::QualType(Type& type) : type(type) {};

QualType QualType::to_val() {
    ref = 0;
    return *this;
}
QualType QualType::to_ref() {
    ref = 1;
    return *this;
}
QualType QualType::to_mut() {
    mut = 1;
    return *this;
}
QualType QualType::to_const() {
    mut = 0;
    return *this;
}
QualType QualType::to_optional() {
    opt = 1;
    return *this;
}
QualType QualType::to_notoptional() {
    opt = 0;
    return *this;
}
bool QualType::is_ref()
{
    return ref==1;
}
bool QualType::is_mut() {
    return mut == 1;
}
bool QualType::is_opt() {
    return opt == 1;
}
