SHELL = /bin/sh

DEFOBJ = deffile.o fgets.o strcmp.o index.o memlist.o
SLDOC = vamps -I sldoc

CFLAGS	= -g -O2 -O3 -ffast-math -march=native -flto
CC = gcc
RANLIB = ranlib
AR = ar
SLINC = ../../../slang/src
INC	= -I../include -I..
DEFS	= -DHAVE_CONFIG_H
RM = rm -f

all: ${DEFOBJ} libdef.a

deffile.o: deffile.c deffile.h
	${CC} ${CFLAGS} $(INC) -c ${DEFS} deffile.c

fgets.o: fgets.c
	${CC} ${CFLAGS} $(INC) -c ${DEFS} fgets.c

index.o: index.c
	${CC} ${CFLAGS} $(INC) -c ${DEFS} index.c

strcmp.o: strcmp.c
	${CC} ${CFLAGS} $(INC) -c ${DEFS} strcmp.c

memlist.o: memlist.c
	${CC} ${CFLAGS} $(INC) -c ${DEFS} memlist.c

doc: 0def.tex
0def.tex: *.c
	${SLDOC} -KC -c *.c > doc
	${SLDOC} -KF -c *.[ch] > fdoc
	${SLDOC} -KI -c *.[ch] > sldoc
	${SLDOC} -i fdoc | sort > sldoc.idx
	${SLDOC} -i doc | sort >> sldoc.idx
	${SLDOC} -i sldoc | sort >> sldoc.idx
	${SLDOC} fdoc doc > 0def.txt
	echo "\\documentclass{article}\\usepackage{makeidx}\\makeindex\\begin{document}\\printindex" > 0def.tex
	echo "\\section{libdef.a}" >> 0def.tex
	${SLDOC} -l fdoc doc >> 0def.tex
	echo "\\end{document}" >> 0def.tex
	${SLDOC} -t fdoc doc > 0def.trf
	rm doc fdoc sldoc.idx

test:	test.c libdef.a
	$(CC) $(CFLAGS) -o test test.c -L. -ldef

clean:
	${RM} ./example ${TRASH} 0def.??? test?.out test?.idx test

libdef.a: $(DEFOBJ) dummy
	${AR} ru $@ $(DEFOBJ)
	${RANLIB} $@

dummy:

