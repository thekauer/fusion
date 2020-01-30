#include "type.h"

static const IntegralType i8_ty = IntegralType(IntegralType::I8,1);
static const IntegralType i8_ty = IntegralType(IntegralType::I16,2);
static const IntegralType i8_ty = IntegralType(IntegralType::I32,4);
static const IntegralType i8_ty = IntegralType(IntegralType::I64,8);

static const IntegralType i8_ty = IntegralType(IntegralType::U8,1);
static const IntegralType i8_ty = IntegralType(IntegralType::U16,2);
static const IntegralType i8_ty = IntegralType(IntegralType::U32,4);
static const IntegralType i8_ty = IntegralType(IntegralType::U64,8);

Type::Type(By pass,bool mut,bool optional) : pass(pass),mut(mut),optional(optional){}

Type* Type::toVal() {
    pass = Val;
    return this;
}
Type* Type::toRef() {
    pass = Ref;
    return this;
}

Type* Type::toPtr() {
    pass = Ptr;
    return this;
}
Type* Type::toMut() {
    mut=true;
    return this;
}

Type* Type::toConst() {
    mut=false;
    return this;
}
Type* Type::toOptional() {
    optional=true;
    return this;
}
Type* Type::toNotOption() {
    optional=false;
    return this;
}

const Type* Type::getI8() {
    return &i8_ty;
}
const Type* Type::getI16() {
    return &i16_ty;
}
const Type* Type::getI32() {
    return &i32_ty;
}

const Type* Type::getI64() {
    return &i64_ty;
}
const Type* Type::getISize() {
    return &isize_ty;
}


const Type* Type::getU8() {
    return &u8_ty;
}
const Type* Type::getU16() {
    return &u16_ty;
}
const Type* Type::getU32() {
    return &u32_ty;
}
const Type* Type::getU64() {
    return &u64_ty;
}
const Type* Type::getUSize() {
    return &usize_ty;
}

const Type* Type::getChar() {
    return &i8_ty;
}
const Type* Type::getBool() {
    return &i32_ty;
}



