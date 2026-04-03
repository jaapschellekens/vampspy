#Makefile for vamps demo stuff
SHELL = /bin/sh

CC	= gcc
CFLAGS	= -g -O2

all: demo

demo: demo.slx
	echo "#!$(BINDIR)/vamps -I " > $@
	cat demo.slx >> $@

clean:
	${RM} ${TRASH} fiji.out aglerr.log soiltab vamps.log

distclean: clean
