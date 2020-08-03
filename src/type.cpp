#include "type.h"
#include "error.h"

Type::Type(const std::string_view name, TypeKind tk, const unsigned int size)
    : name(name), tk(tk), size(size) {}

static const IntegralType i8 = IntegralType("i8", IntegralType::I8, 1);
static const IntegralType i16 = IntegralType("i16", IntegralType::I16, 2);
static const IntegralType i32 = IntegralType("i32", IntegralType::I32, 4);
static const IntegralType i64 = IntegralType("i64", IntegralType::I64, 8);
static const IntegralType isize = IntegralType("isize", IntegralType::ISize, 4);
static const IntegralType u8 = IntegralType("u8", IntegralType::U8, 1);
static const IntegralType u16 = IntegralType("u16", IntegralType::U16, 2);
static const IntegralType u32 = IntegralType("u32", IntegralType::U32, 4);
static const IntegralType u64 = IntegralType("u64", IntegralType::U64, 8);
static const IntegralType usize = IntegralType("usize", IntegralType::USize, 4);
static const IntegralType bool_ = IntegralType("bool", IntegralType::Bool, 1);
static const IntegralType f32 = IntegralType("f32", IntegralType::F32, 4);
static const IntegralType f64 = IntegralType("f64", IntegralType::F64, 8);

unsigned int Type::get_size() const { return size; }

std::string_view Type::get_name() const { return name; }

Type::TypeKind Type::get_typekind() const { return tk; }

const IntegralType &Type::get_i8() { return i8; }

const IntegralType &Type::get_i16() { return i16; }

const IntegralType &Type::get_i32() { return i32; }

const IntegralType &Type::get_i64() { return i64; }

const IntegralType &Type::get_isize() { return isize; }
const IntegralType &Type::get_u8() { return u8; }

const IntegralType &Type::get_u16() { return u16; }

const IntegralType &Type::get_u32() { return u32; }

const IntegralType &Type::get_u64() { return u64; }

const IntegralType &Type::get_usize() { return usize; }

const IntegralType &Type::get_char() { return u8; }

const IntegralType &Type::get_bool() { return bool_; }

const IntegralType &Type::get_f32() { return f32; }
const IntegralType &Type::get_f64() { return f64; }
const IntegralType &Type::get_string() {
  Error::ImplementMe("implement integral string type");
}

IntegralType::IntegralType(const std::string_view name, Ty ty,
                           const unsigned int size)
    : ty(ty), Type(name, Integral, size) {}

llvm::Type *IntegralType::codegen(FusionCtx &ctx) const {
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
  case String:
    Error::ImplementMe("implement string literal code generation");
  default:
    return nullptr;
  } /*
 if (mods &= Ptr) {
   ret->getPointerTo();
 }*/
  return ret;
}

QualType::QualType(const Type &type) : type(&type){};
QualType::QualType(const IntegralType &type)
    : type(&static_cast<const Type &>(type)) {}

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
bool QualType::is_ref() const { return ref == 1; }
bool QualType::is_mut() const { return mut == 1; }
bool QualType::is_opt() const { return opt == 1; }

const Type &QualType::get_type() const { return *type; }
const Type *QualType::get_type_ptr() const { return type; }

QualType &QualType::operator=(const QualType &other) {
  type = other.type;
  mods = other.mods;
  return *this;
}


unsigned int StructType::get_struct_size() {
    unsigned int sum = 0;
    for (const auto& t : fields) {
        sum += t.get_type_ptr()->get_size();
    }
    return sum;
}
llvm::Type* StructType::codegen(FusionCtx& ctx) const {
    std::vector<llvm::Type*> tys;
    for (const auto& t : fields) {
        tys.push_back(t.get_type_ptr()->codegen(ctx));
    }
    return llvm::StructType::create(ctx.ctx,llvm::ArrayRef(tys),llvm::StringRef(name.data()),false);
}