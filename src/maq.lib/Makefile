# Generated automatically from Makefile.in by configure.
SHELL = /bin/sh

CC	= gcc
CFLAGS	= -g -O2

OBJ = marquard.o memory.o func.o strins.o
AR = ar
RANLIB = ranlib
SLDOC = vamps -I sldoc
RM 	= rm -f
EXAMPLE = example
INC	= -I../include -I..
DEFS 	= -DHAVE_CONFIG_H

libmaq.a: ${OBJ} 
	${AR} ru $@ $?
	${RANLIB} $@

marquard.o: marquard.c
	${CC} $(CFLAGS) $(DEFS) $(INC)   -c marquard.c

strins.o: strins.c
	${CC} $(CFLAGS) $(DEFS) $(INC)  -c strins.c

memory.o: memory.c
	${CC} $(CFLAGS) $(DEFS) $(INC)   -c memory.c

func.o: func.c
	${CC} $(CFLAGS) $(DEFS) $(INC)   -c func.c

mex.o: mex.c
	${CC} $(CFLAGS) $(DEFS) $(INC)   -c mex.c

${EXAMPLE}: ${OBJ}  mex.o
	${CC} $(CFLAGS) $(DEFS) $(INC)  ${OBJ} mex.o -o $@  -lslang -lm 

doc: 0maq.tex
0maq.tex: *.c
	${SLDOC} -KC -c *.c > doc
	${SLDOC} -i doc | sort > sldoc.idx
	${SLDOC} doc > 0maq.txt
	echo "\\documentclass{article}\\usepackage{makeidx}\\makeindex\\begin{document}\\printindex" > 0maq.tex
	echo "\\section{libmaq.a}" >> 0maq.tex
	${SLDOC} -l doc >> 0maq.tex
	echo "\\end{document}" >> 0maq.tex
	${SLDOC} -t doc > 0maq.trf
	rm doc sldoc.idx

clean:
	${RM} ${TRASH} ${EXAMPLE} 0maq.??? ${MAQ}  libmaq.a

