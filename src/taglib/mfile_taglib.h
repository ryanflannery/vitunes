/*
 * Copyright (c) 2016 Ryan Flannery <ryan.flannery@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef MFILE_TAGLIB_H
#define MFILE_TAGLIB_H

#include "../mfile.h"

/*
 * Scan the file passed for any media meta information using TagLib and return
 * the results in a newly allocated mfile object.
 */
mfile*
taglib_extract(const char* realpath);

/*
 * Take the given mfile object and store the media meta information contained
 * within it to the mfile's path.
 * Returns 0 on success. On failure, it's an int. See constants below for
 * possible values.
 */
int
taglib_save_tags(const mfile* mfile);

/* possible rcodes to taglib_save_tags */
#define MFILE_TAGLIB_NO_SUCH_FILE -1
#define MFILE_TAGLIB_NO_TAGS      -2

#endif
