#pragma once
#include <string>
#include <vector>


class Type {
public:
    enum By : unsigned char { Ref, Ptr, Val } pass;
    enum TypeKind :unsigned char {
        Integral,
        Struct,
        Tuple
    } tk;
    bool mut;
    bool optional;
    const unsigned int size;
    Type *toVal();
    Type *toPtr();
    Type *toRef();
    Type *toMut();
    Type *toConst();
    Type *toOptional();
    Type *toNotOption();
    Type(TypeKind tk,By pass, bool mut, const unsigned int size, bool optional = false);

    static const Type *getI8();
    static const Type *getI16();
    static const Type *getI32();
    static const Type *getI64();
    static const Type *getISize();

    static const Type *getU8();
    static const Type *getU16();
    static const Type *getU32();
    static const Type *getU64();
    static const Type *getUSize();

    static const Type *getChar();
    static const Type *getBool();
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
        Bool
    } ty;
    IntegralType(Ty ty, const unsigned int size);
};

struct StructType : Type {
    struct TypedValue {
        Type *ty;
        std::string name;
    };
    std::vector<TypedValue> members;
    StructType(const std::vector<TypedValue> &members);
};

struct TupleType : Type {
    std::vector<Type *> members;
    TupleType(const std::vector<Type *> &members);
};
