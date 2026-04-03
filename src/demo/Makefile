# Generated automatically from Makefile.in by configure.
# -*- sh -*-

#---------------------------------------------------------------------------
# 1. Choose a C compiler.  It must understand prototypes.
#-----------------------------------------------------------------------------
CC = gcc
CFLAGS = -g -O -fno-strength-reduce
#-----------------------------------------------------------------------------
# 3. Location where object files are placed (Absolute path)
#-----------------------------------------------------------------------------
OBJDIR = /home/hkl/hydro/promo/schj/src/vamps_0.99h/src/demo/objs
SRCDIR = /home/hkl/hydro/promo/schj/src/vamps_0.99h/src/demo
#-----------------------------------------------------------------------------
# 2. Directory where the various libraries are located.
#-----------------------------------------------------------------------------
SLANG_INCLUDE = $(SRCDIR)/../src#             location of slang.h
SLANG_LIB = $(SLANG_INCLUDE)/$(ARCH)objs#     location of libslang.a

#---------------------------------------------------------------------------
# 3. Other libraries
#---------------------------------------------------------------------------
TCAPLIB = 
#TCAPLIB = -ltermcap

#----------------------------------------------------------------------------
# End of user configuration
#----------------------------------------------------------------------------

SHELL = /bin/sh
OTHERSTUFF = array simple useropen complex pager keypad smgtest
CONFIG_H = config.h

ALL_CFLAGS = $(CFLAGS) -Dunix -I$(SLANG_INCLUDE)

COMPILE_CMD = $(CC) -c $(ALL_CFLAGS)
EXECLIBS = -L$(SLANG_LIB) -lslang -lm $(TCAPLIB)
EXECDEPS = $(SLANG_LIB)/libslang.a
DOT_O_DEPS = demolib.c

#---------------------------------------------------------------------------
all: $(OBJDIR) $(CONFIG_H) $(OTHERSTUFF)

$(CONFIG_H) : $(SLANG_LIB)/libslang.a ../src/config.h
	/bin/cp ../src/config.h $(CONFIG_H)
$(SLANG_LIB)/libslang.a :
	cd ../src; $(MAKE)
#
$(OBJDIR):
	@mkdir $(OBJDIR)
#---------------------------------------------------------------------------
# Housekeeping
#---------------------------------------------------------------------------
clean:
	-/bin/rm -f $(OBJDIR)/* *~
distclean: clean
	-/bin/rm -rf $(OBJDIR) Makefile
#---------------------------------------------------------------------------
# Everything else from configure script
#---------------------------------------------------------------------------

