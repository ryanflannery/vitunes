#include <gtest/gtest.h>

#include <iostream>
using namespace std;

extern "C" {
#  include "mfile_taglib.c"
};

const char *TestFileRO = "taglib/test_files/winamp.mp3";
const char *TestFileRW = "taglib/test_files/winamp_mod.mp3";

/* Utilities */

/*
 * This returns a COPY of what SHOULD be extracted from the TestFileRO
 * (the read-only test file).
 * It does NOT extract that info -- rather it explicitly builds it, so we
 * can test the extraction.
 */
mfile*
get_static_winamp_mfile_info()
{
   mfile *m = mfile_new();
   m->filename = strdup("/some/random/path");
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

TEST(mfile_taglib, taglib_extract)
{
   mfile *dynamic   = taglib_extract("taglib/test_files/winamp.mp3");
   mfile *reference = get_static_winamp_mfile_info();

   ASSERT_NE((mfile*)NULL, dynamic);
   ASSERT_NE((mfile*)NULL, reference);

   EXPECT_TRUE(mfile_cmp(dynamic, reference));
   EXPECT_TRUE(mfile_cmp(reference, dynamic));

   mfile_free(dynamic);
   mfile_free(reference);
}

TEST(mfile_taglib, taglib_save_tags)
{
}
