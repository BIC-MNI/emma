# -------------------------------------------------------------------------
# Makefile for the file(s) in the emma/source/libsource directory. 
#
# $Id: Makefile,v 1.3 1997-10-17 20:22:11 greg Exp $

EMMA_ROOT = ../../

include $(EMMA_ROOT)Makefile.site


# --------------------------------------------------------------------
# Definitions: directories, compiler flags, files, etc.

CFLAGS    = $(CFLAGS_STD) $(DEFINES) $(INCLUDES)
LINTFLAGS = $(LINTOPT) $(INCLUDES)
HEADERS   = $(EMMAINC)/ParseArgv.h \
            $(EMMAINC)/emmageneral.h \
	    $(EMMAINC)/mexutils.h \
            $(EMMAINC)/mierrors.h \
            $(EMMAINC)/mincutil.h \
            $(EMMAINC)/time_stamp.h

LIB = $(EMMALIB)/libemma.a

LIBSRC = mincutil.c \
         createnan.c \
         mexutils.c \
         intframes.c \
         lookup12.c \
         monotonic.c \
         trapint.c \
         ParseArgv.c \
         time_stamp.c
LIBOBJ = $(LIBSRC:.c=.o)

# --------------------------------------------------------------------
# Targets

all: $(LIB)

$(LIB): $(LIBOBJ)
	ar ur $@ $?
	@if test -n "$(RANLIB)" ; then echo "$(RANLIB) $@" ; $(RANLIB) $@ ; fi

dep: .depend
	@echo "header dependencies up to date."

.depend: $(LIBSRC)
	$(CC) -M $(INCLUDES) $(LIBSRC) > .depend

clean:
	rm -f *.o $(EMMALIB)/libemma.a

include .depend
