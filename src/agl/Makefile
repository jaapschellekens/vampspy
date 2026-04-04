SHELL = /bin/sh


CC	= gcc
CFLAGS	= -g -O2 -ffast-math -march=native
INC	= ..
AGLOBJ	= 
OBJDIR	= ../objsDarwin_arm64
INC	= -I../include -I..
DEFS	= -DHAVE_CONFIG_H

all:	$(AGLOBJ)

${OBJDIR}/agl_sl.o: agl_sl.c
	${CC} ${CFLAGS} ${INC} -c ${DEFS} -o $@ agl_sl.c
