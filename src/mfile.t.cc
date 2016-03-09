#include <gtest/gtest.h>

extern "C" {
#  include "mfile.c"
};

const mfile TestMfile1 = {
   (char*)"filename",
   false,

   (char*)"artist",
   (char*)"album",
   (char*)"title",
   (char*)"comment",
   (char*)"genre",
   1981,
   33,

   4,
   101,
   102,
   103,

   0,
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

TEST(mfile, mfile_cmp)
{
   mfile *left  = mfile_new();
   mfile *right = mfile_new();
}
