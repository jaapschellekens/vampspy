SHELL = /bin/sh

CC	= gcc
CFLAGS	= -g -O2 -ffast-math -march=native
TOBJ	= ${OBJDIR}/intopsys.o  ${OBJDIR}/notree.o ${OBJDIR}/topout.o ${OBJDIR}/pre_can.o ${OBJDIR}/canopy.o
OBJDIR	= ../objsDarwin_arm64
INC	= -I../include -I..
DEFS	= -DHAVE_CONFIG_H -DHAVE_LIBPYTHON
SLDOC	= vamps -I sldoc
RM 	= rm -f


all: ${TOBJ}

${OBJDIR}/intopsys.o: intopsys.c topsys.h
	${CC} ${CFLAGS} $(INC) -c ${DEFS} -o $@ intopsys.c 

${OBJDIR}/canopy.o: canopy.c topsys.h
	${CC} ${CFLAGS} $(INC) -c ${DEFS} -o $@ canopy.c 

${OBJDIR}/notree.o: notree.c topsys.h
	${CC} ${CFLAGS} $(INC) -c ${DEFS} -o $@ notree.c 

${OBJDIR}/topout.o: topout.c topsys.h
	${CC} ${CFLAGS} $(INC) -c ${DEFS} -o $@ topout.c 

${OBJDIR}/pre_can.o: pre_can.c topsys.h
	${CC} ${CFLAGS} $(INC) -c ${DEFS} -o $@ pre_can.c 


${OBJDIR}/fullcan.o: fullcan.c topsys.h
	${CC} ${CFLAGS} $(INC) -c ${DEFS} -o $@ fullcan.c 


doc: 0topsys.tex
0topsys.tex: *.c
	${SLDOC} -KC -c *.c > doc
	${SLDOC} -i doc | sort > sldoc.idx
	${SLDOC} doc > 0topsys.txt
	echo "\\documentclass{article}\\usepackage{makeidx}\\makeindex\\begin{document}\\printindex" > 0topsys.tex
	echo "\\section{topsys}" >> 0topsys.tex
	${SLDOC} -l doc >> 0topsys.tex
	echo "\\end{document}" >> 0topsys.tex
	${SLDOC} -t doc > 0topsys.trf
	rm doc sldoc.idx

