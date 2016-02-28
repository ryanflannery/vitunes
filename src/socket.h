/*
 * Copyright (c) 2011 Kilian Klimek <kilian.klimek@gmail.com>
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
#ifndef SOCKET_H
#define SOCKET_H

#include <sys/types.h>

#define VITUNES_RUNNING "WHOWASPHONE?"

/*
 * send (null terminated) msg to vitunes. Returns 0 on success,
 * -1 on errors.
 */
int sock_send_msg(const char *msg);

/*
 * open vitunes socket for listening. Returns a socket on success,
 * -1 on errors.
 */
int sock_listen(void);

/*
 * Receive message from sock into msg. A maximum number of msg_len
 * bytes will be stored. msg will be null terminated. Returns -1 on
 * errors.
 */
ssize_t sock_recv_msg(int sock, char *msg, size_t msg_len);

/*
 * Receive message and process it.
 */
void sock_recv_and_exec(int sock);

#endif /* SOCKET_H */
