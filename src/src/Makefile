# Generated automatically from Makefile.in by configure.
# -*- sh -*-
#---------------------------------------------------------------------------
# ANSI C compiler
#---------------------------------------------------------------------------
CC              = gcc
CFLAGS 		= -g -O -fno-strength-reduce
LDFLAGS		= 
#---------------------------------------------------------------------------
# ELF specific (Linux)
#---------------------------------------------------------------------------
ELF_CC 		= gcc
ELF_CFLAGS	= -O2 -fno-strength-reduce -fPIC
ELF_LINK        = gcc -shared -Wl,-lm,-t,-soname#
#---------------------------------------------------------------------------
# Set these values to ABSOLUTE path names
#---------------------------------------------------------------------------
SRCDIR = /home/hkl/hydro/promo/schj/src/vamps_0.99h/src/src# Location of sources
OBJDIR = /home/hkl/hydro/promo/schj/src/vamps_0.99h/src/src/objs# Location of objects
ELFDIR = /home/hkl/hydro/promo/schj/src/vamps_0.99h/src/src/elfobjs# Location of elf objects

#---------------------------------------------------------------------------
# Set for termcap support
#TCAPLIB = -ltermcap
TCAPLIB = 
#---------------------------------------------------------------------------
# Directory where library is going to go when installed
#---------------------------------------------------------------------------
prefix = /usr
exec_prefix = ${prefix}
install_lib_dir = ${exec_prefix}/lib
install_include_dir = ${prefix}/include

#---------------------------------------------------------------------------
# Misc comamnds (safe to leave these untouched)
#---------------------------------------------------------------------------
RANLIB 		= ranlib
INSTALL		= ../slang.0.99/autoconf/install.sh -c
INSTALL_DATA	= ${INSTALL} -m 644
MKINSDIR	= ../autoconf/mkinsdir.sh
RM 		= rm -f
RM_R		= rm -rf
AR_CR 		= ar cr
RMDIR 		= rmdir
LN 		= ln -sf
CP 		= cp
MKDIR		= mkdir

#---------------------------------------------------------------------------
# There should be no need to change anything below here.
#---------------------------------------------------------------------------
THIS_LIB		= slang#
OTHERSTUFF 		= 
THIS_LIB_DEFINES	= -DSLANG
ELF_MAJOR_VERSION	= 0#
ELF_MINOR_VERSION	= 99.38#

CONFIG_H		= config.h
ALL_CFLAGS		= $(CFLAGS) -Dunix $(THIS_LIB_DEFINES)
ALL_ELF_CFLAGS		= $(ELF_CFLAGS) -Dunix $(THIS_LIB_DEFINES)
COMPILE_CMD		= $(CC) -c $(ALL_CFLAGS)
ELFCOMPILE_CMD		= $(ELF_CC) -c $(ALL_ELF_CFLAGS)

NORMAL_LIB = lib$(THIS_LIB).a
OBJDIR_NORMAL_LIB = $(OBJDIR)/$(NORMAL_LIB)

ELFLIB = lib$(THIS_LIB).so#
ELFLIB_MAJOR = $(ELFLIB).$(ELF_MAJOR_VERSION)#
ELFLIB_MAJOR_MINOR = $(ELFLIB).$(ELF_MAJOR_VERSION).$(ELF_MINOR_VERSION)#

ELFDIR_ELF_LIB = $(ELFDIR)/$(ELFLIB_MAJOR_MINOR)#

EXECLIBS = -L$(OBJDIR) -lslang -lm $(TCAPLIB)
EXECDEPS = $(OBJDIR_NORMAL_LIB)

OFILES = 
OBJS = 
ELFOBJS = 

SHELL = /bin/sh
#---------------------------------------------------------------------------
# Rules
#---------------------------------------------------------------------------
all: $(OBJDIR) $(CONFIG_H) $(OBJDIR_NORMAL_LIB) $(OTHERSTUFF)
elf: $(ELFDIR) $(CONFIG_H) $(ELFDIR_ELF_LIB)

$(OBJDIR_NORMAL_LIB): $(OBJDIR) $(OBJS)
	-$(RM) $(OBJDIR_NORMAL_LIB)
	cd $(OBJDIR); $(AR_CR) $(NORMAL_LIB) $(OFILES)
	$(RANLIB) $(OBJDIR_NORMAL_LIB)
	@echo ""
	@echo $(NORMAL_LIB) created in $(OBJDIR)
# The following was removed below:
#	cd $(ELFDIR); $(RM) $(ELFLIB_MAJOR); $(LN) $(ELFLIB_MAJOR_MINOR) $(ELFLIB_MAJOR)
$(ELFDIR_ELF_LIB): $(ELFDIR) $(ELFOBJS)
	-$(RM) $(ELFDIR_ELF_LIB)
	cd $(ELFDIR); $(ELF_LINK),$(ELFLIB_MAJOR) -o $(ELFLIB_MAJOR_MINOR) $(OFILES)
	cd $(ELFDIR); $(RM) $(ELFLIB); $(LN) $(ELFLIB_MAJOR) $(ELFLIB)
	@echo ""
	@echo $(ELFLIB_MAJOR_MINOR) created in $(ELFDIR).
	@echo The link $(ELFLIB) to $(ELFLIB_MAJOR_MINOR) was also created.
	@echo ""
$(OBJDIR) :
	-$(MKDIR) $(OBJDIR)
$(ELFDIR) :
	-$(MKDIR) $(ELFDIR)
$(CONFIG_H) : sysconf.h
	-$(CP) sysconf.h $(CONFIG_H)

#---------------------------------------------------------------------------
# Intallation rules
#---------------------------------------------------------------------------
install: $(install_lib_dir) $(install_include_dir) $(OBJDIR_NORMAL_LIB)
	@echo installing $(OBJDIR_NORMAL_LIB) in $(install_lib_dir)
	$(INSTALL_DATA) $(OBJDIR_NORMAL_LIB) $(install_lib_dir)
	$(RANLIB) $(install_lib_dir)/$(NORMAL_LIB)
	chmod 644 $(install_lib_dir)/$(NORMAL_LIB)
	@echo installing slang.h and slcurses.h in $(install_include_dir)
	$(INSTALL_DATA) $(SRCDIR)/slang.h $(install_include_dir)
	$(INSTALL_DATA) $(SRCDIR)/slcurses.h $(install_include_dir)
$(install_include_dir):
	$(MKINSDIR) $(install_include_dir)
$(install_lib_dir):
	$(MKINSDIR) $(install_lib_dir)
install-elf: install elf $(install_lib_dir) $(install_include_dir) $(ELFDIR_ELF_LIB)
	$(INSTALL_DATA) $(SRCDIR)/slang.h $(install_include_dir)
	$(INSTALL_DATA) $(SRCDIR)/slcurses.h $(install_include_dir)
	-$(RM) $(install_lib_dir)/$(ELFLIB)
	-$(RM) $(install_lib_dir)/$(ELFLIB_MAJOR)
	@echo installing $(ELFLIB_MAJOR_MINOR) in $(install_lib_dir)
	$(INSTALL_DATA) $(ELFDIR_ELF_LIB) $(install_lib_dir)
	@echo creating symbolic links to $(ELFLIB_MAJOR_MINOR)
	-cd $(install_lib_dir); $(LN) $(ELFLIB_MAJOR_MINOR) $(ELFLIB)
	@echo ""
	@echo $(ELFLIB_MAJOR_MINOR) created in $(install_lib_dir).
	@echo The link $(ELFLIB) to $(ELFLIB_MAJOR_MINOR) was also created.
	@echo ""
install-links:
	-$(RM) $(install_lib_dir)/$(ELFLIB)
	-$(RM) $(install_lib_dir)/$(ELFLIB_MAJOR)
	cd $(install_lib_dir); $(LN) $(ELFLIB_MAJOR_MINOR) $(ELFLIB_MAJOR)
	cd $(install_lib_dir); $(LN) $(ELFLIB_MAJOR) $(ELFLIB)
	@echo
	@echo A link $(ELFLIB_MAJOR) to $(ELFLIB_MAJOR_MINOR) was created.
	@echo A link $(ELFLIB) to $(ELFLIB_MAJOR) was created.
	@echo ""
#---------------------------------------------------------------------------
# Housekeeping
#---------------------------------------------------------------------------
clean:
	-$(RM) $(OBJDIR)/* *~ $(ELFDIR)/*
distclean: clean
	-$(RM_R) $(OBJDIR) $(ELFDIR) Makefile sysconf.h $(CONFIG_H)
#---------------------------------------------------------------------------
# Object Rules : These are created from the configure script (hopefully)
#---------------------------------------------------------------------------

worm : $(OBJDIR)/worm
	@echo worm created in $(OBJDIR)
$(OBJDIR)/worm : $(OBJDIR)/worm.o $(worm_DEPS) $(EXECDEPS)
	$(CC) -o $(OBJDIR)/worm $(LDFLAGS) $(OBJDIR)/worm.o $(worm_LIBS) $(EXECLIBS)
$(OBJDIR)/worm.o : $(SRCDIR)/worm.c $(DOT_O_DEPS) $(worm_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(worm_INC) $(EXECINC) $(SRCDIR)/worm.c

calc : $(OBJDIR)/calc
	@echo calc created in $(OBJDIR)
$(OBJDIR)/calc : $(OBJDIR)/calc.o $(calc_DEPS) $(EXECDEPS)
	$(CC) -o $(OBJDIR)/calc $(LDFLAGS) $(OBJDIR)/calc.o $(calc_LIBS) $(EXECLIBS)
$(OBJDIR)/calc.o : $(SRCDIR)/calc.c $(DOT_O_DEPS) $(calc_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(calc_INC) $(EXECINC) $(SRCDIR)/calc.c

untic : $(OBJDIR)/untic
	@echo untic created in $(OBJDIR)
$(OBJDIR)/untic : $(OBJDIR)/untic.o $(untic_DEPS) $(EXECDEPS)
	$(CC) -o $(OBJDIR)/untic $(LDFLAGS) $(OBJDIR)/untic.o $(untic_LIBS) $(EXECLIBS)
$(OBJDIR)/untic.o : $(SRCDIR)/untic.c $(DOT_O_DEPS) $(untic_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(untic_INC) $(EXECINC) $(SRCDIR)/untic.c


