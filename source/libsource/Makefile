# -------------------------------------------------------------------------
# Makefile for the file(s) in the emma/source/libsource directory. 

EMMA_ROOT = ../..

include $(EMMA_ROOT)/Makefile.site

OPT       = $(STD_OPT) -c

#
# Change the following depending on what operating system you have
#

# --------------------------------------------------------------------

CFLAGS    = $(OPT) $(INCLUDES)
D_CFLAGS  = $(OPT) $(INCLUDES) -debug 
LINTFLAGS = $(LINTOPT) $(INCLUDES)
HEADERS   = $(EMMAINC)/ParseArgv.h \
            $(EMMAINC)/emmageneral.h \
	    $(EMMAINC)/mexutils.h \
            $(EMMAINC)/mierrors.h \
            $(EMMAINC)/mincutil.h \
            $(EMMAINC)/time_stamp.h

# --------------------------------------------------------------------



all: Makefile $(HEADERS) mexutils.o mincutil.o ParseArgv.o intframes.o lookup12.o time_stamp.o monotonic.o trapint.o $(EMMALIB)/libemma.a

$(EMMALIB)/libemma.a: mincutil.o ParseArgv.o time_stamp.o createnan.o \
                      mexutils.o intframes.o lookup12.o monotonic.o trapint.o \
                      Makefile
	ar ur $@ mincutil.o ParseArgv.o time_stamp.o createnan.o \
                 mexutils.o intframes.o lookup12.o monotonic.o trapint.o
	$(RANLIB) $@

mexutils.o: mexutils.c $(EMMAINC)/mexutils.h
#	$(CC) $(CFLAGS) $< 

mincutil.o: mincutil.c $(EMMAINC)/mincutil.h $(EMMAINC)/mierrors.h
#	$(CC) $(CFLAGS) $< 

ParseArgv.o: ParseArgv.c $(EMMAINC)/ParseArgv.h
#	$(CC) $(CFLAGS) $< 

intframes.o: intframes.c
#	$(CC) $(CFLAGS) $<

lookup12.o: lookup12.c
#	$(CC) $(CFLAGS) $<

time_stamp.o: time_stamp.c $(EMMAINC)/time_stamp.h
#	$(CC) $(CFLAGS) $<

monotonic.o: monotonic.c
#	$(CC) $(CFLAGS) $<

trapint.o: trapint.c
#	$(CC) $(CFLAGS) $<


#$(EMMALIB)/libmexemma.a:
#	ar ur $@ mexutils.o intframes.o lookup12.o monotonic.o trapint.o
#	$(RANLIB) $@

clean:
	rm -f *.o $(EMMALIB)/libemma.a
