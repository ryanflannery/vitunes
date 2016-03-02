#include <gtest/gtest.h>

extern "C" {
#	include "str2argv.c"
};

/*
 * utility functions
 */
void
print_argv(int argc, char *argv[])
{
   fprintf(stderr, "dumping array of size %i\n", argc);
   for (int i = 0; i < argc; i++)
      fprintf(stderr, "argv[%i] = '%s'\n", i, argv[i]);
}

bool
argvs_match(int argc1, char *argv1[],
            int argc2, char *argv2[])
{
   if (argc1 != argc2)
   {
      fprintf(stderr, "array sizes differ: %i vs %i\n", argc1, argc2);
      return false;
   }

   for (int i = 0; i < argc1; i++)
   {
      if (strlen(argv1[i]) != strlen(argv2[i])
      ||  strcmp(argv1[i], argv2[i]))
      {
         fprintf(stderr, "element %i differs: '%s' vs '%s'\n",
            i, argv1[i], argv2[i]);
         return false;
      }
   }

   return true;
}

/*
 * sample data used below
 */

char *ARGV_FooBar[] = {
      (char*) "foo",
      (char*) "bar"
};
const int ARGC_FooBar = sizeof(ARGV_FooBar) / sizeof(char*);

char *ARGV_MaryHadALittleLamb [] = {
      (char*) "mary",
      (char*) "had",
      (char*) "a",
      (char*) "little",
      (char*) "lamb"
};
const int ARGC_MaryHadALittleLamb = sizeof(ARGV_MaryHadALittleLamb) / sizeof(char*);

/*
 * testing argv2str
 *
 *    Going FROM arrays(argvs) TO strings
 */

TEST(argv2str, TestEmpty)
{
   ASSERT_EQ(NULL, argv2str(0, NULL));
}

TEST(argv2str, TestMaryHadALittleLamb)
{
   ASSERT_STREQ("mary had a little lamb ",
                argv2str(ARGC_MaryHadALittleLamb, ARGV_MaryHadALittleLamb));
}

TEST(argv2str, TestSizeTooSmall)
{
   ASSERT_STREQ("foo", argv2str(ARGC_FooBar - 1, ARGV_FooBar));
}

TEST(argv2str, TestSizeJustRight)
{
   ASSERT_STREQ("foo bar ", argv2str(ARGC_FooBar, ARGV_FooBar));
}

TEST(argv2str, TestSizeTooBig)
{
   ASSERT_STREQ("foo", argv2str(ARGC_FooBar - 1, ARGV_FooBar));
}

/*
 * testing str2argv
 *
 *    Going FROM strings TO arrays(argvs)
 */

TEST(str2argv, TestNull)
{
   ASSERT_EQ(1, str2argv((char*)"some test string", NULL, NULL, NULL));
}

TEST(str2argv, TestBasic)
{
   char  *input = (char*)"mary had a little lamb";

   int    argc;
   char **argv;
   const char *error;

   int ret = str2argv(input, &argc, &argv, &error);

   ASSERT_EQ(0, ret);
   ASSERT_EQ(ARGC_MaryHadALittleLamb, argc);
   ASSERT_EQ(true, argvs_match(argc, argv,
         ARGC_MaryHadALittleLamb, ARGV_MaryHadALittleLamb));
}

