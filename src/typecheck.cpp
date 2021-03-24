#include "parser.h"

void AstExpr::type_check() const {}
void Body::type_check() const {
  for (const auto &line : body) {
    line->type_check();
  }
}
void Stmt::type_check() const {}
void Expr::type_check() const {}

void VarDeclExpr::type_check() const {}
void FnProto::type_check() const {}

bool has_return(AstExpr *ast) {
  switch (ast->ast_type) { 
    case AstType::Body: {
    for (const auto &line : ast->cast<Body>()->body) {
        has_return(line.get());
    }
    break;
    }
    case AstType::ReturnStmt: {
        return true;
    }
    default: {
    }

  }
    return false;
}

void FnDecl::type_check() const {
  proto->type_check();
  for (const auto &line : body->body) {
    line->type_check();
  }
  
  //temporary check, if main doesnt have return statement signal an error, will be removed later for main
  if (proto->name == "main" && !has_return(body.get())) {
    Error::MainNoReturn(proto->sl);
  }

}

void ValExpr::type_check() const {}
void VarExpr::type_check() const {}
void TypeExpr::type_check() const {}
void FnCall::type_check() const {}
void BinExpr::type_check() const { 
    const Type *left_type = nullptr;
    const Type *right_type = nullptr;
    if (lhs->ast_type == AstType::VarDeclExpr) {
        left_type = lhs->cast<VarDeclExpr>()->type.get_type_ptr();
    }
    if (lhs->ast_type == AstType::VarExpr) {
        left_type = lhs->cast<VarDeclExpr>()->type.get_type_ptr();
    }
    if (rhs->ast_type == AstType::ValExpr) {
      right_type = rhs->cast<ValExpr>()->val.type.get_type_ptr();
    }

    if (left_type && right_type &&
        left_type->get_name() != right_type->get_name()) {
      Error::NoConversionExists(sl, left_type->get_name(),
                                right_type->get_name());
    }

}
void RangeExpr::type_check() const {}
void IfStmt::type_check() const {}
void ReturnStmt::type_check() const {}
void ImportExpr::type_check() const {}
void ClassStmt::type_check() const {}