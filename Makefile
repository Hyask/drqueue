# $Id: Makefile,v 1.15 2001/07/17 10:03:42 jorge Exp $

CC = gcc
OBJS_LIBDRQUEUE = computer_info.o computer_status.o task.o logger.o communications.o \
			computer.o request.o semaphores.o job.o drerrno.o database.o
LDFLAGS =

ifeq ($(systype),linux)
	CFLAGS = -Wall -I. -D__LINUX -g
else 
 ifeq ($(systype),irix)
	CFLAGS = -Wall -I. -D__IRIX -g
 endif
endif

.PHONY: clean irix linux tags
linux: 
	make systype=linux all
irix:
	/usr/freeware/bin/gmake systype=irix all

all: slave master sendjob

libdrqueue.a : $(OBJS_LIBDRQUEUE) libdrqueue.h
	ar sq $@ $(OBJS_LIBDRQUEUE)
slave: slave.o libdrqueue.a
master: master.o libdrqueue.a
sendjob: sendjob.o libdrqueue.a

libdrqueue.h: computer_info.h computer_status.h task.h logger.h communications.h \
			computer.h request.h semaphores.h job.h drerrno.h database.h

%.o: %.c %.h constants.h
	$(CC) -c $(CFLAGS) -o $@ $<
clean:
	rm -f *.o *~ libdrqueue.a slave master sendjob TAGS

tags:
	etags *.[ch] drqman/*.[ch]
