SHELL = /bin/sh

CC	= gcc
CFLAGS	= -g -O2 -ffast-math -march=native
SOBJ	= ${OBJDIR}/soilout.o ${OBJDIR}/swatsoil.o ${OBJDIR}/integral.o ${OBJDIR}/timestep.o ${OBJDIR}/soilboun.o ${OBJDIR}/headcalc.o ${OBJDIR}/soilut.o ${OBJDIR}/band.o ${OBJDIR}/calcgwl.o ${OBJDIR}/fluxes.o ${OBJDIR}/setzero.o ${OBJDIR}/rootex.o ${OBJDIR}/det_hatm.o ${OBJDIR}/reduceva.o ${OBJDIR}/filltab.o ${OBJDIR}/misc_p.o ${OBJDIR}/smooth.o ${OBJDIR}/alloc.o ${OBJDIR}/array.o ${OBJDIR}/getparm.o
OBJDIR	= ../objsDarwin_arm64
INC	= -I../include -I..
DEFS	= -DHAVE_CONFIG_H -DHAVE_LIBPYTHON
SLDOC	= vamps -I sldoc
RM 	= rm -f


all:	${SOBJ}

${OBJDIR}/nhc.o: nhc.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ nhc.c

nhc: ${OBJDIR}/nhc.o
	cc -L ../../lib -o nhc ${OBJDIR}/nhc.o -lnru -lm


${OBJDIR}/alloc.o: alloc.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ alloc.c

${OBJDIR}/getparm.o: getparm.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ getparm.c

${OBJDIR}/array.o: array.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ array.c

${OBJDIR}/band.o: band.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ band.c
	
${OBJDIR}/smooth.o: smooth.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ smooth.c

${OBJDIR}/soilout.o: soilout.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ soilout.c

${OBJDIR}/thet_cal.o: thet_cal.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ thet_cal.c

${OBJDIR}/thet_it.o: thet_it.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ thet_it.c

${OBJDIR}/filltab.o: filltab.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ filltab.c

${OBJDIR}/vangenu.o: vangenu.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ vangenu.c

${OBJDIR}/det_hatm.o: det_hatm.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ det_hatm.c

${OBJDIR}/misc_p.o: misc_p.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ misc_p.c

${OBJDIR}/reduceva.o: reduceva.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ reduceva.c

${OBJDIR}/rootex.o: rootex.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ rootex.c

${OBJDIR}/setzero.o: setzero.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ setzero.c

${OBJDIR}/fluxes.o: fluxes.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ fluxes.c

${OBJDIR}/calcgwl.o: calcgwl.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ calcgwl.c

${OBJDIR}/integral.o: integral.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ integral.c

${OBJDIR}/timestep.o: timestep.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ timestep.c

${OBJDIR}/headcalc.o: headcalc.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ headcalc.c

${OBJDIR}/richard.o: richard.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ richard.c


${OBJDIR}/swatsoil.o: swatsoil.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ swatsoil.c

${OBJDIR}/soilboun.o: soilboun.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ soilboun.c

${OBJDIR}/soilut.o: soilut.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ soilut.c

${OBJDIR}/stdvangenu: vangenu.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -DSTANDALONE vangenu.c -o stdvgn.o -I../deffile -I../include

${OBJDIR}/vangenu: stdvangenu
	${CC} ${DEFS}  -DSTANDALONE stdvgn.o ../util/getopt.o -L../../lib -o ../vangenu -ldeffile -lts -lm

doc: 0soil.tex
0soil.tex: *.c
	${SLDOC} -KC -c *.c > doc
	${SLDOC} -i doc | sort > sldoc.idx
	${SLDOC} doc > 0soil.txt
	echo "\\documentclass{article}\\usepackage{makeidx}\\makeindex\\begin{document}\\printindex" > 0soil.tex
	echo "\\section{soil}" >> 0soil.tex
	${SLDOC} -l doc >> 0soil.tex
	echo "\\end{document}" >> 0soil.tex
	${SLDOC} -t doc > 0soil.trf
	rm doc sldoc.idx
