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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "socket.h"
#include "commands.h"


int
sock_send_msg(const char *path, const char *msg)
{
   int                  ret;
   struct sockaddr_un   addr;

   if((ret = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
      return -1;

   addr.sun_family = AF_UNIX;
   if (strlcpy(addr.sun_path, path, sizeof addr.sun_path) >= sizeof addr.sun_path) {
      errno = ENAMETOOLONG;
      return -1;
   }

   if (sendto(ret, msg, strlen(msg), 0, (struct sockaddr *) &addr, sizeof addr) == -1) {
      close(ret);
      return -1;
   }

   close(ret);
   return 0;
}


int
sock_listen(const char *path)
{
   int                  ret;
   struct sockaddr_un   addr;
   int                  coe = 1;

   unlink(path);

   if((ret = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
      return -1;

   addr.sun_family = AF_UNIX;
   if (strlcpy(addr.sun_path, path, sizeof addr.sun_path) >= sizeof addr.sun_path) {
      errno = ENAMETOOLONG;
      return -1;
   }

   if (bind(ret, (struct sockaddr *) &addr, sizeof addr) == -1)
      return -1;

   fcntl(ret, F_SETFD, FD_CLOEXEC, &coe);

   return ret;
}


ssize_t
sock_recv_msg(int sock, char *msg, size_t msg_len)
{
   struct sockaddr_un   addr;
   socklen_t            addr_len = sizeof addr;
   ssize_t              ret;

   ret = recvfrom(sock, msg, msg_len, 0, (struct sockaddr *) &addr, &addr_len);

   if(ret > -1)
      msg[ret] = '\0';

   return ret;
}


void
sock_recv_and_exec(int sock)
{
   char   msg[64];

   if(sock_recv_msg(sock, msg, sizeof(msg)) == -1)
      return;

   if(!strcmp(msg, VITUNES_RUNNING))
      return;

   if(!kb_execute_by_name(msg))
      cmd_execute(msg);
}
