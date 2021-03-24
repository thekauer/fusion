#include "gtest/gtest.h"
#include "type.h"

TEST(types, IntegralType) {
  auto type = QualType(Type::get_i8()).to_mut().to_optional();
  EXPECT_EQ(type.get_type_ptr()->get_typekind(), Type::Integral);
  EXPECT_TRUE(type.is_opt());
  EXPECT_TRUE(type.is_mut());
}

TEST(types, resolve_type) { 
  auto type = QualType(ResolveType("A"));
  EXPECT_EQ(type.get_type_ptr()->get_typekind(),Type::Resolve);
}
