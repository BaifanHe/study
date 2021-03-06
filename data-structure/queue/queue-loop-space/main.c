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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "queue.h"


int 
main(int argc, char* argv[])
{
  int i;
  double* d;
  queue_t q = queue_create(0);

  fprintf(stdout, "queue length is : %d\n", queue_size(q));
  assert(queue_empty(q));
  assert(0 == queue_size(q));

  srand(time(0));
  for (i = 0 ; i < 8; ++i) {
    d = (double*)malloc(sizeof(double));
    *d = rand() % 2324 * 0.153;
    fprintf(stdout, "queue enqueue element value is : %lf\n", *d);
    queue_enqueue(q, d);
    assert(i + 1 == queue_size(q));
  }
  fprintf(stdout, "queue length is : %d\n", queue_size(q));

  while (!queue_empty(q)) {
    d = (double*)queue_dequeue(q);
    fprintf(stdout, "queue dequeue element value is : %lf\n", *d);
    free(d);
  }
  fprintf(stdout, "queue length is : %d\n", queue_size(q));

  queue_delete(&q);
  return 0;
}
