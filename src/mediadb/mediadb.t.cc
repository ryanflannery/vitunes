#include <gtest/gtest.h>

extern "C" {
#  include "mediadb.c"
};

#include <unistd.h>

const char *TEST_DB = "/tmp/test_vitunes_db.db";

TEST(mediadb, mediadb_open_fail)
{
   const char *nodb = "/path/to/something/that/doesnt/exist";

   /* twice is intentional - want to ensure it doesn't create the db */
   EXPECT_EQ(NULL, mdb_open(nodb));
   EXPECT_EQ(NULL, mdb_open(nodb));
}

TEST(mediadb, mediadb_init)
{
   /* if it exists, remove TEST_DB */
   if (!access(TEST_DB, F_OK))
      ASSERT_EQ(0, unlink(TEST_DB));

   mediadb *mdb = mdb_init(TEST_DB);
   EXPECT_TRUE(NULL != mdb);
   mdb_close(mdb);
}

TEST(mediadb, mediadb_init_fail_if_file_exists)
{
   EXPECT_EQ(NULL, mdb_init(TEST_DB));
}

TEST(mediadb, mediadb_open_succeed)
{
   mediadb *mdb = mdb_open(TEST_DB);
   ASSERT_TRUE(NULL != mdb);
   mdb_close(mdb);
}

TEST(mediadb, mediadb_scan_file)
{
   mediadb *mdb = mdb_open(TEST_DB);
   ASSERT_TRUE(NULL != mdb);
   mdb_scan_file(mdb, "mfile/taglib/test_files/winamp.mp3");
   mdb_scan_file(mdb, "mfile/taglib/test_files/winamp.mp3");
   mdb_scan_file(mdb, "mfile/taglib/test_files/winamp_mod.mp3");
   mdb_scan_file(mdb, "mfile/taglib/test_files/winamp_mod.mp3");
   mdb_close(mdb);
}

TEST(mediadb, mediadb_scan_dirs)
{
   char *dirs[2] = { (char*)"/home/ryan/music", NULL };

   mediadb *mdb = mdb_open(TEST_DB);
   ASSERT_TRUE(NULL != mdb);
   mdb_scan_dirs(mdb, dirs);
   mdb_close(mdb);
}

TEST(mediadb, mediadb_rescan_files)
{
   mediadb *mdb = mdb_open(TEST_DB);
   ASSERT_TRUE(NULL != mdb);
   mdb_rescan_files(mdb);
   mdb_close(mdb);
}

TEST(mediadb, mediadb_extract_filenames)
{
   mediadb *mdb = mdb_open(TEST_DB);
   ASSERT_TRUE(NULL != mdb);
   extract_files(mdb);
   mdb_close(mdb);
}
