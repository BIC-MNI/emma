#
# Top-level Makefile for EMMA.
#
# Changes made here should propagate to lower level makefiles.
# The only change you should make to this file is EMMA_ROOT; other
# site-specific options are in Makefile.site.
#
# $Id: Makefile,v 1.2 1997-10-09 21:14:20 greg Exp $
#

#
# Change this to reflect the EMMA root directory (usually the directory
# where this Makefile lives)
#

EMMA_ROOT   = /usr/people/wolforth/work/emma

#
# Include site-specific and architecture-specific definitions.
# (Makefile.site *must* be edited in order for EMMA to compile
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


# Prepare for and build a distribution.

include Makefile.version

distprep: 
	cd doc ; $(MAKE)

rcsname:
	@for file in `cat MANIFEST` ; do \
	  echo "rcs -q -N$(RCSNAME): -s$(STATE) $$file" ; \
	  rcs -q -N$(RCSNAME): -s$(STATE) $$file ; \
	done

dist: distprep
	mkdir $(RELEASE)
	perl5 -MExtUtils::Manifest=maniread,manicopy \
	  -e '$$mani = maniread;' \
	  -e 'manicopy ($$mani, "$(RELEASE)", "best");
	gtar czf $(ARCHIVE) $(RELEASE)
	rm -rf $(RELEASE)
