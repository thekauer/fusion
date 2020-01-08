#include "type.h"


IntegralType::IntegralType(Type_e type,bool mut=false,By_e by=Ref) :
 type(type),mut(mut),by(by) {}

constexpr const size_t IntegralType::size() const{
    switch (type)
    {
    case Char:
        return 1;
    case String:
        return 4;//fix later
    case I32:
        return 4;
    case I16:
        return 2;
    case I64:
        return 8;
    case Float:
        return sizeof(float);
    case Double:
        return sizeof(double);
    case USize:
        return sizeof(ptr);
    case U8:
        return 1;
    case U16:
        return 2;
    case U32:
        return 4;
    case U64:
        return 4;
    default:
        break;
    }
}

constexpr bool IntegralType::sign() const{
    return type>=USize;
}

const size_t Type::size() {
    int s=0;
    for(const auto& f : fields) {
        s+=f.size();
    }
    return s;
}


Lit::Lit(int v) {
    llvm::Type* ty = llvm::IntegerType::get(ctx,32);
    val =llvm::ConstantInt::get(ty,v,true);
}