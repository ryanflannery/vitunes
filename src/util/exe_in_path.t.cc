#include <gtest/gtest.h>

extern "C" {
#  include "exe_in_path.h"
};

TEST(exe_in_path, TestNull)
{
   ASSERT_EQ(false, exe_in_path(NULL));
}

TEST(exe_in_path, TestEmptyString)
{
   ASSERT_EQ(false, exe_in_path(""));
}

TEST(exe_in_path, TestLS)
{
   ASSERT_EQ(true, exe_in_path("ls"));
}

TEST(exe_in_path, TestNotInPath)
{
   ASSERT_EQ(false, exe_in_path("SomethingDefinitelyNotInPath"));
}

TEST(exe_in_path, TestRandomChars)
{
   ASSERT_EQ(false, exe_in_path("*!03/;]asdf$FOO"));
}
