#include <gtest/gtest.h>

extern "C" {
#  include "str2argv.h"
};

TEST(argv2str, TestEmpty)
{
   ASSERT_EQ(NULL, argv2str(0, NULL));
}

TEST(argv2str, TestSizeTooSmall)
{
   char *argv[2] = { (char*)"foo", (char*)"bar" };
   ASSERT_STREQ("foo", argv2str(1, argv));
}

TEST(argv2str, TestSizeJustRight)
{
   char *argv[2] = { (char*)"foo", (char*)"bar" };
   ASSERT_STREQ("foo bar ", argv2str(2, argv));
}

TEST(argv2str, TestSizeTooBig)
{
   char *argv[3] = { (char*)"foo", (char*)"bar", (char*)"baz" };
   ASSERT_STREQ("foo bar ", argv2str(2, argv));
}

TEST(argv2str, TestNonZeroSizeWithNULL)
{
   ASSERT_STREQ(NULL, argv2str(10, NULL));
}
