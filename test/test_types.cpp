#include "gtest/gtest.h"
#include "type.h"

TEST(types, resolve_type) { 
  auto ty = QualType(ResolveType("A"));
  EXPECT_EQ(ty.get_type_ptr()->get_typekind(),Type::Resolve);
}