#
# Top-level Makefile for the EMMA Matlab Medical Analysis package.
#
# Changes made here should propagate to lower level make files.
# The only change you should make to this file is EMMA_ROOT; other
# site-specific options are in Makefile.site.
#

#
# Change this to reflect the EMMA root directory (usually the directory
# where this Makefile lives)
#

EMMA_ROOT   = /usr/people/wolforth/work/emma

#
# Include site-specific and architecture-specific definitions.
# (Makefile.site *must* be editied in order for EMMA to compile
# properly!)
#

include Makefile.site

SHELL       = /bin/sh

#
# Where to find the source for the various standalone and CMEX programs.
#

C_SOURCES   = $(EMMA_ROOT)/source



######################################################
#                                                    #
# Shouldn't need to touch anything after this point. #
#                                                    #
######################################################


#export RANLIB MEX_EXT CMEX_LIBS XDR_LIB CC CMEX_OPT STD_OPT

CMEX_TARGETS = delaycorrect lookup miinquire mireadimages mireadvar \
               nfmins nframeint ntrapz rescale

C_TARGETS    = bloodtonc bldtobnc includeblood micreateimage \
               miwriteimages miwritevar

TARGETS      = $(CMEX_TARGETS) $(C_TARGETS)


default: all

#
# Make sure that we make the EMMA library first.
#

all : emmalibrary
	@echo making all
	cd $(C_SOURCES); for d in $(TARGETS); do \
		if test -d $$d; then (cd $$d; $(MAKE) $(makeargs)); \
	else true; fi; done

emmalibrary:
	@echo making library
	cd $(C_SOURCES)/libsource; $(MAKE) $(makeargs)

install:
	if [ ! -d $(BIN_INSTALL_DIR) ] ; \
	  then mkdir $(BIN_INSTALL_DIR) ; fi
	if [ ! -d $(MATLAB_INSTALL_DIR) ] ; \
	  then mkdir $(MATLAB_INSTALL_DIR) ; fi
	cd bin ; cp $(C_TARGETS) $(BIN_INSTALL_DIR)
	cd matlab ; for d in general rcbf fdg roi; do cp $$d/* $(MATLAB_INSTALL_DIR) ; done

clean:
	rm -f `find . \( -name \*.o -o -name \*.$(MEX_EXT) -o -name lib\*.a \) -print` bin/*
