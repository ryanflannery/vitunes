#include <gtest/gtest.h>

#include <iostream>
using namespace std;

extern "C" {
#  include "mfile_taglib.c"
};

/* Utilities */

mfile*
test_mfile_get_winamp_mfile()
{
   mfile *m = mfile_new();
   m->filename = strdup("taglib/test_files/winamp.mp3");
   m->artist   = strdup("DJ Mike Llama");
   m->album    = strdup("Beats of Burdon");
   m->title    = strdup("Llama Whippin' Intro");
   m->comment  = strdup("");
   m->genre    = strdup("Rock");
   m->year     = 0;
   m->track    = 1;
   m->length   = 5;
   m->bitrate  = 56;
   m->samplerate = 22050;
   m->channels = 2;
   return m;
}

/* Begin Tests */

TEST(mfile_taglib, MFILE_Extract)
{
   mfile *m = taglib_extract("taglib/test_files/winamp.mp3");

   ASSERT_NE((mfile*)NULL, m);
   const mfile *WINAMP_FILE_INFO = test_mfile_get_winamp_mfile();

   EXPECT_STREQ(m->filename,  "taglib/test_files/winamp.mp3");
   EXPECT_STREQ(m->artist,    "DJ Mike Llama");
   EXPECT_STREQ(m->album,     "Beats of Burdon");
   EXPECT_STREQ(m->title,     "Llama Whippin' Intro");
   EXPECT_STREQ(m->comment,   "");
   EXPECT_STREQ(m->genre,     "Rock");
   EXPECT_EQ(m->year,         0);
   EXPECT_EQ(m->track,        1);
   EXPECT_EQ(m->length,       5);
   EXPECT_EQ(m->bitrate,      56);
   EXPECT_EQ(m->samplerate,   22050);
   EXPECT_EQ(m->channels,     2);

   mfile_free(m);
}

TEST(mfile_taglib, MFILE_Cmp)
{
   mfile *m = taglib_extract("taglib/test_files/winamp.mp3");
   ASSERT_NE((mfile*)NULL, m);
   const mfile *WINAMP_FILE_INFO = test_mfile_get_winamp_mfile();
   ASSERT_STREQ(WINAMP_FILE_INFO->artist, m->artist);
   mfile_fwrite(m, stdout);
   mfile_fwrite(WINAMP_FILE_INFO, stdout);
   EXPECT_TRUE(mfile_cmp(m, WINAMP_FILE_INFO));
}
