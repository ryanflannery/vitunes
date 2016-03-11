#include <gtest/gtest.h>

extern "C" {
#  include "plist.c"
};

static const mfile m[] = {
   { "filename", false, "artist", "album", "title", "comment", "genre", 1981, 33, 1, 2, 3, 4, 0 }
};

TEST(plist, plist_new)
{
   plist *p = plist_new();

   EXPECT_STREQ(NULL, p->filename);
   EXPECT_STREQ(NULL, p->name);
   EXPECT_EQ(NULL, p->mfiles);
   EXPECT_TRUE(0 == p->nfiles);
   EXPECT_TRUE(0 == p->capacity);

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

   EXPECT_EQ(p1->nfiles,   p2->nfiles);
   EXPECT_EQ(p1->capacity, p2->capacity);
}
