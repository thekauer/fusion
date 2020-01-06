#pragma once
#include <vector>
#include "compatibility.h"
#include <memory>
enum By_e :unsigned char{
    Copy,
    Ref,
    Ptr
};

enum Type_e :unsigned char{
    Char,
    String,
    I32,
    I16,
    I64,
    Bool,
    Float,
    Double,
    USize,
    U8,
    U16,
    U32,
    U64
};
struct IntegralType {
bool mut;
By_e by;
Type_e type;
constexpr const size_t size() const;
constexpr bool sign() const;
IntegralType(Type_e type,bool mut,By_e by);

};


struct Type  {
    const std::vector<IntegralType> fields; 
    const size_t size();
    Type(const std::vector<IntegralType> fields) : fields(fields){};
};

struct Lit {
    IntegralType type=IntegralType(I32,false,Copy);
    ptr val;
    Lit(IntegralType type,ptr val):type(type),val(val){};
};



