#pragma once
#include "context.h"
#include <memory>
#include <string>
#include <vector>

struct IntegralType;
class Type {
public:
  enum TypeKind : unsigned char { Integral, Array, Struct, Tuple };
  Type(const std::string_view name, TypeKind tk, const unsigned int size);

  unsigned int get_size() const;
  std::string_view get_name() const;
  TypeKind get_typekind() const;

  std::unique_ptr<Type> to_ptr();

  virtual llvm::Type *codegen(FusionCtx &ctx) const = 0;

  static const IntegralType &get_i8();
  static const IntegralType &get_i16();
  static const IntegralType &get_i32();
  static const IntegralType &get_i64();
  static const IntegralType &get_isize();
  static const IntegralType &get_u8();
  static const IntegralType &get_u16();
  static const IntegralType &get_u32();
  static const IntegralType &get_u64();
  static const IntegralType &get_usize();
  static const IntegralType &get_char();
  static const IntegralType &get_bool();
  static const IntegralType &get_f32();
  static const IntegralType &get_f64();
  static const IntegralType &get_string();

protected:
  TypeKind tk;
  unsigned char mods;
  const unsigned int size;
  std::string name;
};

struct IntegralType : Type {
  enum Ty : unsigned char {
    I8,
    I16,
    I32,
    I64,
    ISize,
    U8,
    U16,
    U32,
    U64,
    USize,
    Char,
    Bool,
    F32,
    F64,
    String
  } ty;
  IntegralType(const std::string_view name, Ty ty, const unsigned int size);
  llvm::Type *codegen(FusionCtx &ctx) const override;
};

class QualType {
public:
  QualType(const Type &type);
  QualType(const IntegralType &type);
  QualType() = default;

  QualType to_val();
  QualType to_ref();
  QualType to_mut();
  QualType to_const();
  QualType to_optional();
  QualType to_notoptional();

  bool is_ref() const;
  bool is_mut() const;
  bool is_opt() const;

  const Type &get_type() const;
  const Type *get_type_ptr() const;

  QualType &operator=(const QualType &other);

private:
  Type const *type{nullptr};
  unsigned char mods;
  unsigned mut : 1;
  unsigned ref : 1;
  unsigned opt : 1;
};

struct StructType : Type {
private:
  static unsigned int get_struct_size(std::vector<QualType> &fields);

public:
  std::vector<QualType> fields;
  StructType(std::string_view name, std::vector<QualType> &&fields)
      : fields(std::move(fields)),
        Type(name, Type::Struct, get_struct_size(fields)){};
  llvm::Type *codegen(FusionCtx &ctx) const override;
};