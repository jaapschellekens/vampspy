SHELL = /bin/sh


CC = gcc
CFLAGS = -g -O2 -O3 -ffast-math -march=native -flto
UOBJ	= ${OBJDIR}/dataset.o ${OBJDIR}/output.o ${OBJDIR}/getopt.o ${OBJDIR}/progress.o ${OBJDIR}/mktable.o ${OBJDIR}/utout.o ${OBJDIR}/perror.o ${OBJDIR}/pro_mesg.o ${OBJDIR}/xtraout.o ${OBJDIR}/resamp.o ${OBJDIR}/memory.o ${OBJDIR}/smoothar.o
OBJDIR	= ../objsDarwin_arm64
INC	= -I../include -I..
DEFS	= -DHAVE_CONFIG_H
SLDOC	= vamps -I sldoc
RM 	= rm -f

all:	${UOBJ} ${OBJDIR}/vsel.o ${OBJDIR}/getstr.o 

${OBJDIR}/smoothar.o: smoothar.c 
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ smoothar.c 

${OBJDIR}/popen.o: popen.c 
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ popen.c 

${OBJDIR}/resamp.o: resamp.c 
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ resamp.c 

${OBJDIR}/nrutil.o: nrutil.c 
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ nrutil.c 

${OBJDIR}/utout.o: utout.c 
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ utout.c 

${OBJDIR}/memory.o: memory.c 
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ memory.c 

${OBJDIR}/xtraout.o: xtraout.c 
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ xtraout.c 

${OBJDIR}/perror.o: perror.c 
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ perror.c 

${OBJDIR}/mktable.o: mktable.c 
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ mktable.c 

${OBJDIR}/mktable: mktable.c 
	${CC} -g -I ../include -L ../../lib -o mktable -DTST  mktable.c -lts

${OBJDIR}/vrep.o: vrep.c 
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ vrep.c 

${OBJDIR}/getstr.o: getstr.c 
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ getstr.c 

${OBJDIR}/vsel.o: vsel.c 
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ vsel.c 

${OBJDIR}/issec.o: issec.c 
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ issec.c 

${OBJDIR}/getopt.o: getopt.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ getopt.c

${OBJDIR}/progress.o: progress.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ progress.c

${OBJDIR}/pro_mesg.o: pro_mesg.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ pro_mesg.c

${OBJDIR}/dataset.o: dataset.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ dataset.c

${OBJDIR}/output.o: output.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ output.c

doc: 0util.tex
0util.tex: *.c
	${SLDOC} -KC -c *.c > doc
	${SLDOC} -i doc | sort > sldoc.idx
	${SLDOC} doc > 0util.txt
	echo "\\documentclass{article}\\usepackage{makeidx}\\makeindex\\begin{document}\\printindex" > 0util.tex
	echo "\\section{util}" >> 0util.tex
	${SLDOC} -l doc >> 0util.tex
	echo "\\end{document}" >> 0util.tex
	${SLDOC} -t doc > 0util.trf
	rm doc sldoc.idx
