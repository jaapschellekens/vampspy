#Makefile for vamps
SHELL = /bin/sh

CC	= gcc
CFLAGS	= -g -O2 -O3 -ffast-math -march=native -flto

CINT =  ../src/main/*.c ../src/maq.lib/*.c\
DOC = ../doc/*.tex 
top_srcdir 	= ../..
srcdir 		= .
prefix 		= /usr/local
exec_prefix 	= ${prefix}
BINDIR	= ${exec_prefix}/bin


clean:
	${RM} ${TRASH} mkidx mkinp slvsel report run pretop demo vampsfit sldoc

distclean: clean
