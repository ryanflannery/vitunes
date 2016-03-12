#include <gtest/gtest.h>

extern "C" {
#  include "mfile.c"
};

TEST(mfile, mfile_new)
{
   mfile *m = mfile_new();

   EXPECT_STREQ(NULL, m->filename);
   EXPECT_STREQ(NULL, m->artist);
   EXPECT_STREQ(NULL, m->album);
   EXPECT_STREQ(NULL, m->title);
   EXPECT_STREQ(NULL, m->comment);
   EXPECT_STREQ(NULL, m->genre);
   EXPECT_EQ(0,       m->year);
   EXPECT_EQ(0,       m->track);
   EXPECT_EQ(0,       m->length);
   EXPECT_EQ(0,       m->bitrate);
   EXPECT_EQ(0,       m->samplerate);
   EXPECT_EQ(0,       m->channels);
   EXPECT_EQ(0,       m->last_update);
   EXPECT_EQ(false,   m->is_url);

   mfile_free(m);
}

TEST(mfile, mfile_free)
{
   mfile *m = mfile_new();
   EXPECT_NO_THROW(mfile_free(m));
}

TEST(mfile, mfile_cmp_with_empty_mfiles)
{
   mfile *left  = mfile_new();
   mfile *right = mfile_new();

   /* reflexive */
   EXPECT_TRUE(mfile_cmp(left,  left));
   EXPECT_TRUE(mfile_cmp(right, right));

   /* symmetric */
   EXPECT_TRUE(mfile_cmp(left, right));
   EXPECT_TRUE(mfile_cmp(right, left));

   mfile_free(left);
   mfile_free(right);
}

TEST(mfile, mfile_construct)
{
   mfile *m1 = mfile_construct("artist", "album", "title", "comment", "genre",
         1981, 33);
   mfile *m2 = mfile_construct("artist", "album", "title", "comment", "genre",
         1981, 33);

   /* reflexive */
   EXPECT_TRUE(mfile_cmp(m1, m1));
   EXPECT_TRUE(mfile_cmp(m2, m2));

   /* symmetric */
   EXPECT_TRUE(mfile_cmp(m1, m2));
   EXPECT_TRUE(mfile_cmp(m2, m1));
}

TEST(mfile, mfile_construct_against_static)
{
   const mfile test =
   {
      (char*)"filename",
      false,

      (char*)"artist",
      (char*)"album",
      (char*)"title",
      (char*)"comment",
      (char*)"genre",
      1981,
      33,

      0,
      0,
      0,
      0,

      0,
   };

   /* same as the TestMfile1 above */
   mfile *m = mfile_construct("artist", "album", "title", "comment", "genre",
         1981, 33);

   EXPECT_TRUE(mfile_cmp(&test, m));
}

