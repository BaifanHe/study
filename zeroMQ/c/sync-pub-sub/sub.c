/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>


int 
main(int argc, char* argv[])
{
  int count = 0;
  char buf[128];
  void* ctx = zmq_ctx_new();
  void* sub = zmq_socket(ctx, ZMQ_SUB);
  void* sync = zmq_socket(ctx, ZMQ_REQ);

  zmq_connect(sub, "tcp://localhost:5555");
  zmq_setsockopt(sub, ZMQ_SUBSCRIBE, "", 0);
  zmq_connect(sync, "tcp://localhost:6666");

  zmq_send(sync, "", 0, 0);
  zmq_recv(sync, buf, sizeof(buf), 0);

  fprintf(stdout, "connected to publish server success ...\n");
  while (1) {
    memset(buf, 0, sizeof(buf));
    zmq_recv(sub, buf, sizeof(buf), 0);
    if (0 == strncmp("END", buf, 3))
      break;
    fprintf(stdout, "recevied : %s\n", buf);
    ++count;
  }
  fprintf(stdout, "recevied count : %d\n", count);

  zmq_close(sync);
  zmq_close(sub);
  zmq_ctx_destroy(ctx);

  return 0;
}
