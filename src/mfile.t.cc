#include <gtest/gtest.h>

extern "C" {
#  include "mfile.c"
};

TEST(mfile_new, MFILE_New_Basic)
{
   mfile *m = mfile_new();

   ASSERT_STREQ(NULL, m->filename);
   ASSERT_STREQ(NULL, m->artist);
   ASSERT_STREQ(NULL, m->album);
   ASSERT_STREQ(NULL, m->title);
   ASSERT_STREQ(NULL, m->comment);
   ASSERT_STREQ(NULL, m->genre);
   ASSERT_EQ(0,       m->year);
   ASSERT_EQ(0,       m->track);
   ASSERT_EQ(0,       m->length);
   ASSERT_EQ(0,       m->bitrate);
   ASSERT_EQ(0,       m->samplerate);
   ASSERT_EQ(0,       m->channels);
   ASSERT_EQ(0,       m->last_update);
   ASSERT_EQ(false,   m->is_url);

   mfile_free(m);
}

TEST(mfile_free, MFILE_Free_Null)
{
   mfile *m = mfile_new();
   mfile_free(m);
}
