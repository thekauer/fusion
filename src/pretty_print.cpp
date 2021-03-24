#include "parser.h"
#include "llvm/Support/raw_os_ostream.h"

void Body::pretty_print() const {
  for (const auto &line : body) {
    for (int i = 0; i < sl.indent; i++)
      llvm::outs() << " ";
    line->pretty_print();
    llvm::outs() << "\n";
  }
}

void FnProto::pretty_print() const {
  llvm::outs() << "fn " << name << "(";
  for (const auto &arg : args) {
    arg->pretty_print();
    llvm::outs() << ",";
  }
  if (args.size() == 0)
    llvm::outs() << "(";
  llvm::outs() << "\b)";
}

void FnDecl::pretty_print() const {
  proto->pretty_print();
  llvm::outs() << "\n";
  body->pretty_print();
  llvm::outs() << "\n";
}

void ValExpr::pretty_print() const {
  switch (val.ty.get_type_ptr()->get_typekind()) {
  case Type::Integral: {
    switch (static_cast<const IntegralType *>(val.ty.get_type_ptr())->ty) {
    case IntegralType::I8:
      llvm::outs() << val.as.i8;
      return;
    case IntegralType::I16:
      llvm::outs() << val.as.i16;
      return;
    case IntegralType::I32:
      llvm::outs() << val.as.i32;
      return;
    case IntegralType::I64:
      llvm::outs() << val.as.i64;
      return;
    case IntegralType::ISize:
      llvm::outs() << val.as.i64;
      return;
    case IntegralType::U8:
      llvm::outs() << val.as.u8;
      return;
    case IntegralType::U16:
      llvm::outs() << val.as.u16;
      return;
    case IntegralType::U32:
      llvm::outs() << val.as.u32;
      return;
    case IntegralType::U64:
      llvm::outs() << val.as.u64;
      return;
    case IntegralType::USize:
      llvm::outs() << val.as.u64;
      return;
    case IntegralType::Bool:
      if (val.as.b) {
        llvm::outs() << "true";
      } else {
        llvm::outs() << "false";
      }
      return;
    }
  }
  default:
    llvm::outs() << "val";
  }
}

void VarDeclExpr::pretty_print() const {
  llvm::outs() << name << " : " << ty.get_type_ptr()->get_name().data();
}
void VarExpr::pretty_print() const { llvm::outs() << name; }
void TypeExpr::pretty_print() const {
  llvm::outs() << ty.get_type_ptr()->get_name().data();
}
void FnCall::pretty_print() const {
  llvm::outs() << name << "(";
  for (const auto &arg : args) {
    if (arg) {
      arg->pretty_print();
      llvm::outs() << ",";
    }
  }
  if (args.size() == 0)
    llvm::outs() << "(";
  llvm::outs() << "\b)\n";
}

void BinExpr::pretty_print() const {
  lhs->pretty_print();
  llvm::outs() << " ";
  switch (op) {
  case Token::Add:
    llvm::outs() << "+";
    break;
  case Token::Eq:
    llvm::outs() << "=";
    break;
  case Token::Mul:
    llvm::outs() << "*";
    break;
  default:
    llvm::outs() << " op ";
    break;
  }
  llvm::outs() << " ";
  rhs->pretty_print();
}

void RangeExpr::pretty_print() const {
  if (begin)
    begin->pretty_print();
  llvm::outs() << "..";
  if (end)
    end->pretty_print();
}

void IfStmt::pretty_print() const {
  llvm::outs() << "if ";
  condition->pretty_print();
  llvm::outs() << "\n";
  body->pretty_print();
  if (else_body) {
    llvm::outs() << "else\n";
    else_body->pretty_print();
  }
}

void ImportExpr::pretty_print() const { llvm::outs() << "import " << module; }

void ReturnStmt::pretty_print() const {
  llvm::outs() << "return ";
  expr->pretty_print();
}

void ClassStmt::pretty_print() const {
  llvm::outs() << "class " << name->name << "\n";
  body->pretty_print();
}