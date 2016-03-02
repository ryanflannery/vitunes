#include <gtest/gtest.h>

extern "C" {
#	include "str2argv.c"
};

/*
 * utility functions
 */
void
print_argv(const char *name, int argc, char *argv[])
{
   fprintf(stderr, "dumping array %s of size %i\n", name, argc);
   for (int i = 0; i < argc; i++)
      fprintf(stderr, "%s[%i] = '%s'\n", name, i, argv[i]);
}

bool
argvs_match(int argc1, char *argv1[],
            int argc2, char *argv2[])
{
   if (argc1 != argc2)
   {
      fprintf(stderr, "array sizes differ: %i vs %i\n", argc1, argc2);
      print_argv("argv1", argc1, argv1);
      print_argv("argv2", argc2, argv2);
      return false;
   }

   for (int i = 0; i < argc1; i++)
   {
      if (strlen(argv1[i]) != strlen(argv2[i])
      ||  strcmp(argv1[i], argv2[i]))
      {
         fprintf(stderr, "element %i differs: '%s' vs '%s'\n",
               i, argv1[i], argv2[i]);
         print_argv("argv1", argc1, argv1);
         print_argv("argv2", argc2, argv2);
         return false;
      }
   }

   return true;
}

/*
 * sample data used below
 */

// test case {"foo","bar"} <=> "foo bar"
char *ARGV_FooBar[] = {
      (char*) "foo",
      (char*) "bar"
};
const int   ARGC_FooBar = sizeof(ARGV_FooBar) / sizeof(char*);
const char *STR_FooBar  = "foo bar";

// test case {"mary","had","a","little","lamb"} <=> "mary had a little lamb"
char *ARGV_MaryHadALittleLamb [] = {
      (char*) "mary",
      (char*) "had",
      (char*) "a",
      (char*) "little",
      (char*) "lamb"
};
const int   ARGC_MaryHadALittleLamb = sizeof(ARGV_MaryHadALittleLamb) / sizeof(char*);
const char *STR_MaryHadALittleLamb  = "mary had a little lamb";

// test case {"1 ","2  ","3   ","4    "} <=> "1 2 '3 4'"
char *ARGV_TestSpaces[] = {
      (char*) "1 ",
      (char*) "2  ",
      (char*) "3   4    "
};
const int   ARGC_TestSpaces = sizeof(ARGV_TestSpaces) / sizeof(char*);
const char *STR_TestSpaces  = "'1 ' '2  ' '3   4    '";

// test case {"a","b c","d","e f g"} <=> "a 'b c' d 'e f g'
char *ARGV_TestQuotes[] = {
      (char*) "a",
      (char*) "b c",
      (char*) "d",
      (char*) "e f g"
};
const int   ARGC_TestQuotes = sizeof(ARGV_TestQuotes) / sizeof(char*);
const char *STR_TestQuotes = "a 'b c' d 'e f g'";

// TODO fuck...i should really make a macro for the above pattern

/*
 * testing argv2str
 *
 *    Going FROM arrays(argvs) TO strings
 */

TEST(argv2str, TestEmpty)
{
   ASSERT_EQ(NULL, argv2str(0, NULL));
}

TEST(argv2str, TestFooBar)
{
   ASSERT_STREQ(STR_FooBar,
                argv2str(ARGC_FooBar, ARGV_FooBar));
}

TEST(argv2str, TestMaryHadALittleLamb)
{
   ASSERT_STREQ(STR_MaryHadALittleLamb,
                argv2str(ARGC_MaryHadALittleLamb, ARGV_MaryHadALittleLamb));
}

TEST(argv2str, TestSpaces)
{
   ASSERT_STREQ(STR_TestSpaces, argv2str(ARGC_TestSpaces, ARGV_TestSpaces));
}

TEST(argv2str, TestQuotes)
{
   ASSERT_STREQ(STR_TestQuotes, argv2str(ARGC_TestQuotes, ARGV_TestQuotes));
}

TEST(argv2str, TestSizeTooSmall)
{
   ASSERT_STREQ("foo", argv2str(ARGC_FooBar - 1, ARGV_FooBar));
}

TEST(argv2str, TestSizeJustRight)
{
   ASSERT_STREQ(STR_FooBar, argv2str(ARGC_FooBar, ARGV_FooBar));
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

TEST(str2argv, TestFooBar)
{
   int    argc;
   char **argv;
   const char *error;

   ASSERT_EQ(0, str2argv(STR_FooBar, &argc, &argv, &error));
   ASSERT_EQ(true, argvs_match(argc, argv,
         ARGC_FooBar, ARGV_FooBar));
}

TEST(str2argv, TestMaryHadALittleLamb)
{
   int    argc;
   char **argv;
   const char *error;

   ASSERT_EQ(0, str2argv(STR_MaryHadALittleLamb, &argc, &argv, &error));
   ASSERT_EQ(true, argvs_match(argc, argv,
         ARGC_MaryHadALittleLamb, ARGV_MaryHadALittleLamb));
}

TEST(str2argv, TestSpaces)
{
   int    argc;
   char **argv;
   const char *error;

   ASSERT_EQ(0, str2argv(STR_TestSpaces, &argc, &argv, &error));
   ASSERT_EQ(true, argvs_match(argc, argv,
         ARGC_TestSpaces, ARGV_TestSpaces));
}

TEST(str2argv, TestQuotes)
{
   int    argc;
   char **argv;
   const char *error;

   ASSERT_EQ(0, str2argv(STR_TestQuotes, &argc, &argv, &error));
   ASSERT_EQ(true, argvs_match(argc, argv,
         ARGC_TestQuotes, ARGV_TestQuotes));
}

