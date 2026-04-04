SHELL = /bin/sh
# Makefile for Vamps 

# You should not need to change anything below this if configure worked the
# way it should... This only happens on Linux en FreeBSD at the moment....
#
# (C)J. Schellekens 1996-1997
#---------------------------------------------------------------------------

PROGRAM = vamps
PROGS = ${PROGRAM} vsel #wrini getstr issec

top_srcdir 	= .
srcdir 		= .
prefix 		= /usr/local
exec_prefix 	= ${prefix}
BINDIR 		= ${exec_prefix}/bin
LIBDIR 		= ${exec_prefix}/lib
BUILDLIBDIR 	= ./../libDarwin_arm64

OBJDIR 		= ./objsDarwin_arm64
VAMPSLIBDIR	= ${prefix}/share/vamps
mandir		= ${prefix}/share/man

CFLAGS 	= -g -O2 -ffast-math -march=native
CC 	= gcc
OS	= Darwin_arm64
RM	= rm -f
INC	= -I./include -I.
LDFLAGS	=  -L${BUILDLIBDIR}

RANLIB	= ranlib
INSTALL	= /usr/bin/install -c
INSTALL_PROGRAM	= $(INSTALL) -c
PYLIBS	= -L/opt/homebrew/opt/python@3.14/Frameworks/Python.framework/Versions/3.14/lib -Wl,-rpath,/opt/homebrew/opt/python@3.14/Frameworks/Python.framework/Versions/3.14/lib -L/opt/homebrew/opt/python@3.14/Frameworks/Python.framework/Versions/3.14/lib/python3.14/config-3.14-darwin -lpython3.14 -ldl -framework CoreFoundation
LIBS	= -lmet -lts -ldef  -lnru -ltermcap $(PYLIBS) -L/opt/homebrew/opt/python@3.14/Frameworks/Python.framework/Versions/3.14/lib -Wl,-rpath,/opt/homebrew/opt/python@3.14/Frameworks/Python.framework/Versions/3.14/lib -L/opt/homebrew/opt/python@3.14/Frameworks/Python.framework/Versions/3.14/lib/python3.14/config-3.14-darwin -lpython3.14 -ldl -framework CoreFoundation  -lm

SUBDIR = util main soil topsys 
#THELIBS = ts.lib/libts.a deffile.lib/libdef.a\
#	  nr_ut.lib/libnru.a met.lib/libmet.a slang.0.99/src/objs/libslang.a
THELIBS = ts.lib/libts.a deffile.lib/libdef.a\
	  nr_ut.lib/libnru.a met.lib/libmet.a

TRASH 	= *~ vamps vsel *.o lib*.a *.log TAGS

# List of object files
AGLOBJ = 
PLOTOBJ = 
TOBJ = ${OBJDIR}/intopsys.o  ${OBJDIR}/notree.o ${OBJDIR}/topout.o ${OBJDIR}/pre_can.o ${OBJDIR}/canopy.o
UOBJ = ${OBJDIR}/dataset.o ${OBJDIR}/output.o ${OBJDIR}/getopt.o ${OBJDIR}/progress.o ${OBJDIR}/mktable.o ${OBJDIR}/utout.o ${OBJDIR}/perror.o ${OBJDIR}/pro_mesg.o ${OBJDIR}/xtraout.o ${OBJDIR}/resamp.o ${OBJDIR}/memory.o ${OBJDIR}/smoothar.o
MOBJ = ${OBJDIR}/vamps.o ${OBJDIR}/sigs.o ${OBJDIR}/init_py.o ${OBJDIR}/plot.o
TOBJ = ${OBJDIR}/intopsys.o  ${OBJDIR}/notree.o ${OBJDIR}/topout.o ${OBJDIR}/pre_can.o ${OBJDIR}/canopy.o
SOBJ = ${OBJDIR}/soilout.o ${OBJDIR}/swatsoil.o ${OBJDIR}/integral.o ${OBJDIR}/timestep.o ${OBJDIR}/soilboun.o ${OBJDIR}/headcalc.o ${OBJDIR}/soilut.o ${OBJDIR}/band.o ${OBJDIR}/calcgwl.o ${OBJDIR}/fluxes.o ${OBJDIR}/setzero.o ${OBJDIR}/rootex.o ${OBJDIR}/det_hatm.o ${OBJDIR}/reduceva.o ${OBJDIR}/filltab.o ${OBJDIR}/misc_p.o ${OBJDIR}/smooth.o ${OBJDIR}/alloc.o ${OBJDIR}/array.o ${OBJDIR}/getparm.o

AGLRT	= ./agl/noagl
PROGVERSION = 
SHORTPROGVERSION = 

#----------------------------------------------------------------------------
#              Targets start here
#
def:
	@echo Makefile for vamps `cat ../0version`
	@echo "-------------------------------------"
	@echo Use one of the following targets:
	@echo 	"lib (libraries vamps needs)"
	@echo	"all (the rest)"
	@echo	"install (install in ${BINDIR}. Only works if you have S-Lang)"
	@echo	"test (test example/1. Only works if you have S-Lang)"
	@echo   ""
	@echo   "Further targets:"
	@echo 	"clean (clean object files)"
	@echo 	"distclean (clean object files and libs)"


all: lib dirs ${PROGS}

dirs: dummy
	$(SHELL) $(srcdir)/mkdirs.sh  ${OBJDIR} ${BUILDLIBDIR}


lib: dirs Makefile ${THELIBS}

scripts: dummy
	(cd $(srcdir)/scripts; $(MAKE))

deffile.lib/libdef.a: dummy
	(cd $(srcdir)/deffile.lib;$(MAKE))
	$(INSTALL_PROGRAM) $@ ${BUILDLIBDIR}
	$(INSTALL_PROGRAM) deffile.lib/deffile.h include

nr_ut.lib/libnru.a: dummy
	(cd $(srcdir)/nr_ut.lib;$(MAKE))
	$(INSTALL_PROGRAM) $@ ${BUILDLIBDIR}
	$(INSTALL_PROGRAM) nr_ut.lib/nrutil.h include


ts.lib/libts.a: dummy
	(cd $(srcdir)/ts.lib;$(MAKE))
	$(INSTALL_PROGRAM) $@ ${BUILDLIBDIR}
	$(INSTALL_PROGRAM) ts.lib/ts.h include

met.lib/libmet.a: dummy
	(cd $(srcdir)/met.lib;$(MAKE))
	$(INSTALL_PROGRAM) $@ ${BUILDLIBDIR}
	$(INSTALL_PROGRAM) met.lib/met.h include

${PROGRAM}:  ${SUBDIR} 
	${CC} ${CFLAGS} -o ${PROGRAM}  ${LDFLAGS} ${MOBJ} ${GROBJ} ${UOBJ} ${TOBJ} ${SOBJ} ${AGLOBJ} ${PLOTOBJ} ${LDDIRS} ${LIBS}

vsel: 
	${CC} ${CFLAGS} -o vsel ${LDDIRS} ${LDFLAGS} ${OBJDIR}/vsel.o ${OBJDIR}/getopt.o -lm -ldef -lts

vangenu: 
	(cd $(srcdir)/soil; ${MAKE}  CC='${CC}' vangenu)

util: dummy
	(cd $(srcdir)/util; $(MAKE))

topsys: dummy
	(cd $(srcdir)/topsys; $(MAKE))

main: dummy
	(cd $(srcdir)/main; $(MAKE))

#environ: dummy
#	(cd $(srcdir)/environ; $(MAKE))

soil: dummy
	(cd $(srcdir)/soil; $(MAKE))

plot: dummy
	(cd $(srcdir)/plot; $(MAKE))

agl: dummy
	(cd $(srcdir)/agl; $(MAKE))

bin: all share
	@echo Making a binary distribution...
	rm -rf ../bin/${OS}
	mkdir  ../bin/${OS}
	${INSTALL_PROGRAM} ${PROGRAM}  ../bin/${OS}
	${INSTALL_PROGRAM} vsel  ../bin/${OS}
	(cd ../../; tar cvf - vamps_${SHORTPROGVERSION}/share/* vamps_${SHORTPROGVERSION}/bin/* vamps_${SHORTPROGVERSION}/0* vamps_${SHORTPROGVERSION}/doc/* vamps_${SHORTPROGVERSION}/examples/* | gzip -9 > vamps_${OS}${SHORTPROGVERSION}.tgz)

share: dummy
	(cd  ../share; $(MAKE) BINDIR="$(BINDIR)")

demo: dummy
	(cd  ../examples/demo; $(MAKE) BINDIR="$(BINDIR)")

ckall: dummy
	ci -u -f -sAlpha -m"Version: " `find -name \*.[ch]`

#man-install: installdirs
#	${INSTALL_PROGRAM} $(srcdir)/../doc/vamps.1 $(mandir)/man1
#	${INSTALL_PROGRAM} $(srcdir)/../doc/vsel.1 $(mandir)/man1
#	${INSTALL_PROGRAM} $(srcdir)/../doc/vamps.5 $(mandir)/man5

man-install: installdirs
	(cd ../doc; make man-install)

install: share  man-install
	for i in $(AGLRT); do ${INSTALL_PROGRAM} $$i  ../share/; done
	${INSTALL_PROGRAM} ${PROGRAM} ${BINDIR}
	${INSTALL_PROGRAM} vsel ${BINDIR}
	${INSTALL_PROGRAM} ../share/vamps.rc ${VAMPSLIBDIR}/.vampsrc
	for i in ../share/*; do ${INSTALL_PROGRAM} $$i  ${VAMPSLIBDIR}; done

test:	
	${INSTALL_PROGRAM} ../share/vampsfit ${BINDIR}
	@echo Now testing the ${PROGRAM} installation.....
	(cd $(srcdir)/../examples/1;${BINDIR}/${PROGRAM} example1.inp; ${MAKE} clean)

installdirs:
	$(SHELL) $(srcdir)/mkdirs.sh $(mandir)/man1 $(mandir)/man5 $(BINDIR) $(VAMPSLIBDIR)

clean:
	${RM} ${TRASH} config.cache config.log config.status
	${RM} -r ${OBJDIR}


distclean: clean
	${RM} -r ${BUILDLIBDIR}
	${RM} -rf ../bin/${OS}
	(cd deffile.lib; ${MAKE} RM='${RM}' TRASH='${TRASH}' clean)
	(cd nr_ut.lib; ${MAKE} RM='${RM}' TRASH='${TRASH}' clean)
	(cd ts.lib; ${MAKE} RM='${RM}' TRASH='${TRASH}' clean)
	(cd met.lib; ${MAKE} RM='${RM}' TRASH='${TRASH}' clean)
	(cd ../share; ${MAKE} RM='${RM}' TRASH='${TRASH}' clean)
	(cd util; ${RM} ${TRASH})
	(cd main; ${RM} ${TRASH})
	(cd soil; ${RM} ${TRASH})
	(cd plot; ${RM} ${TRASH})
	(cd agl; ${RM} ${TRASH})
	(cd topsys; ${RM} ${TRASH})
	(cd ../examples/fiji; ${MAKE} clean)
	(cd ../examples/1; ${MAKE} clean)
	(cd ../examples/speed; ${MAKE} clean)
	(cd ../examples/lys; ${MAKE} clean)
	(cd ../examples/penman; ${MAKE} clean)
	(cd ../examples/vamps_demo; ${MAKE} RM='${RM}' TRASH='${TRASH}' clean)
	${RM} ID vconfig.h
	(cd ..;${RM} `find . -name \*~`)

doc: 
	(cd deffile.lib; ${MAKE} RM='${RM}' TRASH='${TRASH}' doc)
	(cd nr_ut.lib; ${MAKE} RM='${RM}' TRASH='${TRASH}' doc)
	(cd ts.lib; ${MAKE} RM='${RM}' TRASH='${TRASH}' doc)
	(cd met.lib; ${MAKE} RM='${RM}' TRASH='${TRASH}' doc)
	(cd util; ${MAKE} RM='${RM}' TRASH='${TRASH}' doc)
	(cd main; ${MAKE} RM='${RM}' TRASH='${TRASH}' doc)
	(cd soil; ${MAKE} RM='${RM}' TRASH='${TRASH}' doc)
	(cd topsys; ${MAKE} RM='${RM}' TRASH='${TRASH}' doc)
	(cd ../share; ${MAKE} RM='${RM}' TRASH='${TRASH}' doc)


newversion: clean-all
	csh mknewversion.csh

flop:
	(cd ../../; tar cvzf /dev/fd0 vamps_${SHORTPROGVERSION})

dosflop:
	(cd ../../; zip -9 -r vamps.zip vamps_${SHORTPROGVERSION})
	(cd ../../; mcopy vamps.zip a:)

zip:
	(cd ../../; zip -9 -r vamps_${SHORTPROGVERSION}.zip vamps_${SHORTPROGVERSION})


tgz:
	(cd ../../; tar cvf -  vamps_${SHORTPROGVERSION} | gzip -9 >  vamps_${SHORTPROGVERSION}.tgz)

dist:
	(cd ../../; tar cvf - `find vamps_${SHORTPROGVERSION} -type f | grep -v RCS` | gzip -9 >  vamps_${SHORTPROGVERSION}.tgz)

mkid:
	mkid `find soil/ topsys/ agl/ plot/ util/ nr_ut.lib/ deffile.lib/ main/ ts.lib/ met.lib/ include/ -name "*.[ch]"`

dummy:
