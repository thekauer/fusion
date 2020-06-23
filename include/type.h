#pragma once
#include "context.h"
#include <string>
#include <vector>
#include <memory>

class Type {
public:
  enum TypeKind : unsigned char { Integral, Array, Struct, Tuple };
  Type(const std::string_view name, TypeKind tk, const unsigned int size);


  unsigned int get_size();
  std::string_view get_name();
  TypeKind get_typekind();

  std::unique_ptr<Type> to_ptr();

  virtual llvm::Type* codegen(FusionCtx &ctx) = 0;

  static std::unique_ptr<Type> get_i8();
  static std::unique_ptr<Type> get_i16();
  static std::unique_ptr<Type> get_i32();
  static std::unique_ptr<Type> get_i64();
  static std::unique_ptr<Type> get_isize();
  static std::unique_ptr<Type> get_u8();
  static std::unique_ptr<Type> get_u16();
  static std::unique_ptr<Type> get_u32();
  static std::unique_ptr<Type> get_u64();
  static std::unique_ptr<Type> get_usize();
  static std::unique_ptr<Type> get_char();
  static std::unique_ptr<Type> get_bool();
  static std::unique_ptr<Type> get_f32();
  static std::unique_ptr<Type> get_f64();

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
    F64
  } ty;
  IntegralType(const std::string_view name, Ty ty, const unsigned int size);
  llvm::Type* codegen(FusionCtx &ctx) override;
};


    

class QualType {
public:
    QualType(Type& type);

    QualType to_val();
    QualType to_ref();
    QualType to_mut();
    QualType to_const();
    QualType to_optional();
    QualType to_notoptional();
    
    bool is_ref();
    bool is_mut();
    bool is_opt();
private:
    Type& type;
    unsigned char mods;
    unsigned mut : 1;
    unsigned ref : 1;
    unsigned opt : 1;

};