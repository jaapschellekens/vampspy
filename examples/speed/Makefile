# Generated automatically from Makefile.in by configure.
#Makefile for vamps
SHELL = /bin/sh

CC	= gcc
CFLAGS	= -g -O2

CINT = ../src/met.lib/sl_inter.c ../src/main/*.c ../src/maq.lib/*.c\
	../src/deffile.lib/sl_inter.c
DOC = ../doc/*.tex 
top_srcdir 	= ../..
srcdir 		= .
prefix 		= /usr/local
exec_prefix 	= ${prefix}
BINDIR	= ${exec_prefix}/bin

doc: all
all: sldoc.idx progs
progs: sldoc report mkidx mkinp slvsel pretop run demo vampsfit

intrin.doc: $(CINT)
	../src/vamps -I sldoc.slx -c $(CINT) > intrin.doc

sldoc.idx:	intrin.doc *.sl *.slx
	../src/vamps -I sldoc.slx -i intrin.doc  *.sl *.slx | sort > sldoc.idx

vampsfit: vampsfit.slx
	echo "#!$(BINDIR)/vamps -I " > $@
	cat vampsfit.slx >> $@

sldoc: sldoc.slx
	echo "#!$(BINDIR)/vamps -I " > $@
	cat sldoc.slx >> $@

demo: demo.slx
	echo "#!$(BINDIR)/vamps -I " > $@
	cat demo.slx >> $@

pretop: pretop.slx
	echo "#!$(BINDIR)/vamps -I " > $@
	cat pretop.slx >> $@

run: run.slx
	echo "#!$(BINDIR)/vamps -I " > $@
	cat run.slx >> $@

slvsel: slvsel.slx
	echo "#!$(BINDIR)/vamps -I " > $@
	cat slvsel.slx >> $@

report: report.slx
	echo "#!$(BINDIR)/vamps -I " > $@
	cat report.slx >> $@

mkidx: mkidx.slx
	echo "#!$(BINDIR)/vamps -I " > $@
	cat mkidx.slx >> $@

mkinp: mkinp.slx
	echo "#!$(BINDIR)/vamps -I " > $@
	cat mkinp.slx >> $@

clean:
	${RM} ${TRASH} mkidx mkinp slvsel report run pretop demo vampsfit sldoc

distclean: clean
