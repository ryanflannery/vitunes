#include "util/str2argv.t.cc"
#include "util/exe_in_path.t.cc"

int
main(int argc, char *argv[])
{
   ::testing::InitGoogleTest( &argc, argv );
   return RUN_ALL_TESTS();
}
