#pragma once
#include <string>
#include <vector>
#include "context.h"

class Type {
public:
  enum By : unsigned char { Ref, Ptr, Val };
  enum TypeKind : unsigned char { Integral, Struct, Tuple };
  Type *toVal();
  Type *toPtr();
  Type *toRef();
  Type *toMut();
  Type *toConst();
  Type *toOptional();
  Type *toNotOption();
  Type(const std::string& name,TypeKind tk, By pass, bool mut, const unsigned int size,
       bool optional = false);

  unsigned int getSize();
  const std::string& getName();
  TypeKind getTypeKind();
  By getBy();

  virtual llvm::Type* codegen(FusionCtx& ctx)=0;

  static Type *getI8();
  static Type *getI16();
  static Type *getI32();
  static Type *getI64();
  static Type *getISize();
  static Type *getU8();
  static Type *getU16();
  static Type *getU32();
  static Type *getU64();
  static Type *getUSize();
  static Type *getChar();
  static Type *getBool();
  static Type *getDouble();
protected:
  TypeKind tk;
  By pass;
  bool mut;
  bool optional;
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
    Double
  } ty;
  IntegralType(const std::string& name,Ty ty, const unsigned int size);
  llvm::Type* codegen(FusionCtx& ctx)override;
};

struct StructType : Type {
  struct TypedValue {
    Type *ty;
    std::string name;
  };
  std::vector<TypedValue> members;
  StructType(const std::string& name,const std::vector<TypedValue> &members);
  llvm::Type* codegen(FusionCtx& ctx) override;
};

struct TupleType : Type {
  std::vector<Type *> members;
  TupleType(const std::string& name,const std::vector<Type *> &members);
  llvm::Type* codegen(FusionCtx& ctx) override;
};
