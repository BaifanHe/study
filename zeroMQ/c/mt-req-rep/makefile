# Copyright (c) 2013 ASMlover. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list ofconditions and the following disclaimer.
#
#    notice, this list of conditions and the following disclaimer in
#  * Redistributions in binary form must reproduce the above copyright
#    the documentation and/or other materialsprovided with the
#    distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

REP_BIN	= rep 
REQ_BIN	= req 
CC	= gcc 
RM	= rm 
CFLAGS	= -g -O2 -Wall 
LDFLAGS	= -lzmq
REP_OBJS = rep.o
REQ_OBJS = req.o


all: $(REP_BIN) $(REQ_BIN)

clean:
	$(RM) $(REP_BIN) $(REQ_BIN) $(REP_OBJS) $(REQ_OBJS)

$(REP_BIN): $(REP_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) -lpthread

$(REQ_BIN): $(REQ_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(REP_OBJS): %.o: %.c
	$(CC) -o $*.o -c $(CFLAGS) $^

$(REQ_OBJS): %.o: %.c
	$(CC) -o $*.o -c $(CFLAGS) $^
