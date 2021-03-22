#include "gtest/gtest.h"
#include "type.h"

TEST(types, IntegralType) {
  auto ty = QualType(Type::get_i8()).to_mut().to_optional();
  EXPECT_EQ(ty.get_type_ptr()->get_typekind(), Type::Integral);
  EXPECT_TRUE(ty.is_opt());
  EXPECT_TRUE(ty.is_mut());
}

TEST(types, resolve_type) { 
  auto ty = QualType(ResolveType("A"));
  EXPECT_EQ(ty.get_type_ptr()->get_typekind(),Type::Resolve);
}
