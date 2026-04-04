SHELL = /bin/sh

METOBJ =  e0.o eaes.o earo.o gamma.o lambda.o makkink.o\
	ra.o vslope.o penmon.o int.o rn_open.o

CC	= gcc
CFLAGS	= -g -O2 -ffast-math -march=native

SLDOC = vamps -I sldoc
AR = ar
RANLIB = ranlib
SLINC 	= ../../../slang/src
INC	= -I../include -I..
DEFS	= -DHAVE_CONFIG_H
RM 	= rm -f


all: libmet.a

libmet.a: ${METOBJ} dummy
	${AR} ru $@ ${METOBJ}
	${RANLIB} $@

e0.o: e0.c 
	${CC} ${CFLAGS} $(INC) -c ${DEFS} e0.c

rn_open.o: rn_open.c 
	${CC} ${CFLAGS} $(INC) -c ${DEFS} rn_open.c

int.o: int.c 
	${CC} ${CFLAGS} $(INC) -c ${DEFS} int.c

penmon.o: penmon.c 
	${CC} ${CFLAGS} $(INC) -c ${DEFS} penmon.c

eaes.o: eaes.c
	${CC} ${CFLAGS} $(INC) -c ${DEFS} eaes.c

earo.o: earo.c
	${CC} ${CFLAGS} $(INC) -c ${DEFS} earo.c

gamma.o: gamma.c
	${CC} ${CFLAGS} $(INC) -c ${DEFS} gamma.c
	
lambda.o: lambda.c
	${CC} ${CFLAGS} $(INC) -c ${DEFS} lambda.c
	
makkink.o: makkink.c
	${CC} ${CFLAGS} $(INC) -c ${DEFS} makkink.c
	
ra.o: ra.c
	${CC} ${CFLAGS} $(INC) -c ${DEFS} ra.c
	
vslope.o: vslope.c
	${CC} ${CFLAGS} $(INC) -c ${DEFS} vslope.c
	

doc: 0met.tex
0met.tex: *.c
	${SLDOC} -KC -c *.c > doc
	${SLDOC} -i doc | sort > sldoc.idx
	${SLDOC} doc > 0met.txt
	echo "\\documentclass{article}\\usepackage{makeidx}\\makeindex\\begin{document}\\printindex" > 0met.tex
	echo "\\section{libmet.a}" >> 0met.tex
	${SLDOC} -l doc >> 0met.tex
	echo "\\end{document}" >> 0met.tex
	${SLDOC} -t doc > 0met.trf
	rm doc sldoc.idx

clean:
	${RM} ${TRASH} 0met.???

dummy:
