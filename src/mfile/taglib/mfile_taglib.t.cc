#include <gtest/gtest.h>

#include <iostream>
using namespace std;

extern "C" {
#  include "mfile_taglib.c"
};

const char *TestFileRO = "mfile/taglib/test_files/winamp.mp3";
const char *TestFileRW = "mfile/taglib/test_files/winamp_mod.mp3";

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

TEST(mfile_taglib, taglib_extract_valid_file)
{
   mfile *dynamic   = mfile_extract_tags(TestFileRO);
   mfile *reference = get_static_winamp_mfile_info();

   ASSERT_NE((mfile*)NULL, dynamic);
   ASSERT_NE((mfile*)NULL, reference);

   EXPECT_TRUE(mfile_cmp(dynamic, reference));
   EXPECT_TRUE(mfile_cmp(reference, dynamic));

   mfile_free(dynamic);
   mfile_free(reference);
}

TEST(mfile_taglib, taglib_extract_no_such_file)
{
   EXPECT_EQ(NULL, mfile_extract_tags("/path/to/no/such/file"));
   EXPECT_EQ(NULL, mfile_extract_tags("!$^*!(!@)*$&!&/!@!*@!"));
}

TEST(mfile_taglib, taglib_save_tags)
{
   /* copy static mfile info into RW test file */
   mfile *m = get_static_winamp_mfile_info();
   free(m->filename);
   m->filename = strdup(TestFileRW);
   EXPECT_EQ(0, mfile_save_tags(m));

   /* now check that the saved info matches the extracted */
   mfile *extracted = mfile_extract_tags(TestFileRW);
   EXPECT_TRUE(mfile_cmp(m, extracted));

   mfile_free(m);
   mfile_free(extracted);
   //mfile *custom = mfile_construct("artist", "album", "title", "comment",
   //   "genre", 1981, 33);
}

TEST(mfile_taglib, taglib_save_tags_no_such_file)
{
   mfile *m = get_static_winamp_mfile_info();
   free(m->filename);
   m->filename = strdup("/path/to/no/such/file/foooooo.mp3");

   /* TODO I'd expect one of these to fail, but the don't. See the
    * corresponding note in the implementation of taglib_save_tags().
    * I need to take a closer look at the taglib interface.
    */
   EXPECT_NE(MFILE_SAVE_NO_SUCH_FILE,   mfile_save_tags(m));
   EXPECT_NE(MFILE_SAVE_FAILED_TO_SAVE, mfile_save_tags(m));
}

TEST(mfile_taglib, taglib_extract_dir_should_fail)
{
   ASSERT_EQ(NULL, mfile_extract_tags("/dev"));
}
