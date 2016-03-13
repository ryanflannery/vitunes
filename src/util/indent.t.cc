#include <gtest/gtest.h>

extern "C" {
#  include "indent.c"
};

TEST(indent, print_indent)
{
   FILE *dnull = fopen("/dev/null", "rw");
   ASSERT_TRUE(NULL != dnull);

   print_indent(0, ' ', dnull);
   print_indent(10, ' ', dnull);
   print_indent(100, ' ', dnull);

   fclose(dnull);
}

TEST(indent, indent)
{
   FILE *dnull = fopen("/dev/null", "rw");
   ASSERT_TRUE(NULL != dnull);

   indent(0, dnull);
   indent(10, dnull);
   indent(100, dnull);

   fclose(dnull);
}
