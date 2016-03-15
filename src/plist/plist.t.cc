#include <gtest/gtest.h>

extern "C" {
#  include "plist.c"
};

TEST(plist, plist_new)
{
   plist *p = plist_new();

   EXPECT_STREQ(NULL, p->filename);
   EXPECT_STREQ(NULL, p->name);
   EXPECT_EQ(NULL, p->mfiles->records);
   EXPECT_TRUE(0 == p->mfiles->nrecords);
   EXPECT_TRUE(0 == p->mfiles->capacity);

   plist_free(p);
}

TEST(plist, plist_free)
{
   plist *p = plist_new();
   EXPECT_NO_THROW(plist_free(p));
}

TEST(plist, plist_copy_bland)
{
   plist *p1 = plist_new();
   plist *p2 = plist_copy(p1, "filename", "name");

   EXPECT_STREQ("filename", p2->filename);
   EXPECT_STREQ("name", p2->name);

   EXPECT_EQ(p1->mfiles->nrecords, p2->mfiles->nrecords);
   EXPECT_EQ(p1->mfiles->capacity, p2->mfiles->capacity);

   EXPECT_NO_THROW(plist_free(p1));
   EXPECT_NO_THROW(plist_free(p2));
}

TEST(plist, plist_add_files)
{
   plist *p = plist_new();

   mfile *mfiles[3];
   mfiles[0] = mfile_construct("a1", "A1", "t1", "c1", "g1", 1, 2);
   mfiles[1] = mfile_construct("a2", "A2", "t2", "c2", "g2", 3, 4);
   mfiles[2] = mfile_construct("a3", "A3", "t3", "c3", "g3", 5, 6);

   plist_add_files(p, 0, mfiles, 3);
   EXPECT_EQ((size_t)3, p->mfiles->nrecords);

   plist_add_files(p, p->mfiles->nrecords, mfiles, 3);
   EXPECT_EQ((size_t)6, p->mfiles->nrecords);

   plist_add_files(p, 2, mfiles, 3);
   EXPECT_EQ((size_t)9, p->mfiles->nrecords);

   mfile_free(mfiles[0]);
   mfile_free(mfiles[1]);
   mfile_free(mfiles[2]);
   plist_free(p);
}

TEST(plist, plist_remove_files)
{
   plist *p = plist_new();

   mfile *mfiles[3];
   mfiles[0] = mfile_construct("a1", "A1", "t1", "c1", "g1", 1, 2);
   mfiles[1] = mfile_construct("a2", "A2", "t2", "c2", "g2", 3, 4);
   mfiles[2] = mfile_construct("a3", "A3", "t3", "c3", "g3", 5, 6);

   plist_add_files(p, 0, mfiles, 3);
   EXPECT_EQ((size_t)3, p->mfiles->nrecords);

   plist_remove_files(p, 0, 1);
   EXPECT_EQ((size_t)2, p->mfiles->nrecords);

   plist_remove_files(p, 1, 1);
   EXPECT_EQ((size_t)1, p->mfiles->nrecords);

   plist_remove_files(p, 0, 1);
   EXPECT_EQ((size_t)0, p->mfiles->nrecords);

   mfile_free(mfiles[0]);
   mfile_free(mfiles[1]);
   mfile_free(mfiles[2]);
   plist_free(p);
}

TEST(plist, plist_replace_files)
{
   plist *p = plist_new();

   mfile *mfiles[3];
   mfiles[0] = mfile_construct("a1", "A1", "t1", "c1", "g1", 1, 2);
   mfiles[1] = mfile_construct("a2", "A2", "t2", "c2", "g2", 3, 4);
   mfiles[2] = mfile_construct("a3", "A3", "t3", "c3", "g3", 5, 6);

   mfile *replacement = mfile_construct("a", "a", "t", "c", "g", 0, 0);

   plist_add_files(p, 0, mfiles, 3);

   plist_replace_file(p, 0, replacement);
   EXPECT_STREQ("a", ((mfile*)p->mfiles->records[0])->artist);

   plist_replace_file(p, 1, replacement);
   EXPECT_STREQ("a", ((mfile*)p->mfiles->records[0])->artist);

   plist_replace_file(p, 2, replacement);
   EXPECT_STREQ("a", ((mfile*)p->mfiles->records[0])->artist);

   mfile_free(replacement);
   mfile_free(mfiles[0]);
   mfile_free(mfiles[1]);
   mfile_free(mfiles[2]);
   plist_free(p);
}
