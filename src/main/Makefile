SHELL = /bin/sh

CC	= gcc
CFLAGS	= -g -O2
MOBJ	= ${OBJDIR}/vamps.o ${OBJDIR}/sigs.o ${OBJDIR}/init_py.o ${OBJDIR}/plot.o
top_srcdir 	= ..
srcdir 		= .
prefix 		= /usr/local
exec_prefix 	= ${prefix}
BINDIR 		= ${exec_prefix}/bin
LIBDIR 		= ${exec_prefix}/lib

OBJDIR 		= ../objsDarwin_arm64
RM 	= rm -f
VAMPSLIBDIR	= ${prefix}/share/vamps1.0
INC	= -I$(top_srcdir)/include -I$(top_srcdir)
PYINC	= -I/opt/homebrew/opt/python@3.14/Frameworks/Python.framework/Versions/3.14/include/python3.14 -I/opt/homebrew/opt/python@3.14/Frameworks/Python.framework/Versions/3.14/include/python3.14
GNUPLOT	= 
DEFS	= -DHAVE_CONFIG_H -DHAVE_LIBPYTHON -DVAMPSLIB=\"${VAMPSLIBDIR}\" -DGNUPLOT=\"${GNUPLOT}\"

all: ${MOBJ}

${OBJDIR}/vamps.o: vamps.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ vamps.c

${OBJDIR}/plot.o: plot.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ plot.c

${OBJDIR}/sigs.o: sigs.c
	${CC} $(CFLAGS) $(INC) -c ${DEFS} -o $@ sigs.c

${OBJDIR}/init_py.o: init_py.c
	${CC} $(CFLAGS) $(INC) $(PYINC) -c ${DEFS} -o $@ init_py.c

doc: 0main.tex

0main.tex: *.c
	echo "\\documentclass{article}\\usepackage{makeidx}\\makeindex\\begin{document}\\printindex" > 0main.tex
	echo "\\section{main}" >> 0main.tex
	echo "\\end{document}" >> 0main.tex
