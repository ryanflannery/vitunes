#include <gtest/gtest.h>

extern "C" {
#	include "str2argv.c"
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

TEST(argv2str, TestMaryHadALittleLamb)
{
   char *argv[] = {
		(char*) "mary",
		(char*) "had",
		(char*) "a",
		(char*) "little",
		(char*) "lamb"
	};
   ASSERT_STREQ("mary had a little lamb ", argv2str(5, argv));
}
