# Makefile for the CMEX numerical routines in ~greg/src/cmex.
# Currently able to build:
#
#   lookup.mexsg (from lookup12.c, monotonic.c, and lookup.c)
#   ntrapz.mexsg (from trapint.c and ntrapz.c)
#   nframeint.mexsg (from nframeint.c, lookup12.c, monotonic.c, and trapint.o)
#
# Run make without parameters to build all of these.

#PROG     = lookup
#PROG_MEX = $(PROG).mexsg
#PROG_SRC = $(PROG).c
#PROG_OBJ = $(PROG).o
OPT      = -ansi -DMATLAB -G 0
#LINTOPTS = -u
CC       = cc
USERINC  = /usr/people/greg/src/include
USERLIB  = 
INCLUDES = -I/usr/include \
           -I/usr/local/include \
           -I/usr/local/matlab/extern/include \
           -I$(USERINC)
LDFLAGS  = -L/usr/local/matlab/extern/lib/sgi \
           -lmex -lm_G0 -lc_G0 -lmalloc_G0
# --------------------------------------------------------------------

CFLAGS    = $(OPT) $(INCLUDES)
LINTFLAGS = $(LINTOPTS) $(INCLUDES)

# --------------------------------------------------------------------

default: lookup.mexsg ntrapz.mexsg nframeint.mexsg

lookup.mexsg: lookup.o lookup12.o monotonic.o Makefile
	ld -e mex_entry_pt -d -r -x -u mex_entry_pt -o lookup.mexsg \
	lookup.o lookup12.o monotonic.o $(LDFLAGS)

ntrapz.mexsg: ntrapz.o trapint.o Makefile
	ld -e mex_entry_pt -d -r -x -u mex_entry_pt -o ntrapz.mexsg \
	ntrapz.o trapint.o $(LDFLAGS)

nframeint.mexsg: nframeint.o lookup12.o monotonic.o trapint.o Makefile
	ld -e mex_entry_pt -d -r -x -u mex_entry_pt -o nframeint.mexsg \
	nframeint.o lookup12.o monotonic.o trapint.o $(LDFLAGS)


#lint:
#	lint $(LINTFLAGS) $(PROG_SRC)

#debug:
#	cmex  $(PROG_SRC) $(D_CFLAGS) $< $(D_LDFLAGS) 
