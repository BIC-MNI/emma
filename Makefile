#
# Top-level Makefile for EMMA.
#
# Changes made here should propagate to lower level makefiles.
# The only change you should make to this file is EMMA_ROOT; other
# site-specific options are in Makefile.site.
#
# $Id: Makefile,v 1.4 1997-10-21 16:08:21 greg Rel $
#


# 
# EMMA_ROOT must be defined before we include Makefile.site; it's 
# set to an empty string here because the EMMA root directory is
# just the current directory.
# 

EMMA_ROOT   = 

#
# Include site-specific and architecture-specific definitions.
# (Makefile.site *must* be edited in order for EMMA to compile
# properly!)
#

include Makefile.site

#
# Where to find the source for the various standalone and CMEX programs.
#

C_SOURCES   = source



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
	@echo "*** building all C programs..."
	@for d in $(TARGETS); do \
	  if test -d $(C_SOURCES)/$$d; then \
	    echo "** building $$d..." ;\
	    (cd $(C_SOURCES)/$$d; $(MAKE) $(makeargs)) ;\
	  fi ;\
	done

emmalibrary:
	@echo "*** building the EMMA library..."
	cd $(C_SOURCES)/libsource; $(MAKE) $(makeargs)

install:
	if [ ! -d $(BIN_INSTALL_DIR) ] ; \
	  then mkdir $(BIN_INSTALL_DIR) ; fi
	if [ ! -d $(MATLAB_INSTALL_DIR) ] ; \
	  then mkdir $(MATLAB_INSTALL_DIR) ; fi
	cd bin ; cp $(C_TARGETS) $(BIN_INSTALL_DIR)
	cd matlab ; for d in general rcbf fdg roi; do cp $$d/* $(MATLAB_INSTALL_DIR) ; done
	cd doc ; $(MAKE) install

clean:
	rm -f `find . \( -name \*.o -o -name \*.$(MEX_EXT) -o -name lib\*.a \) -print` bin/*


# Prepare for and build a distribution.

include Makefile.version
RCSNAME  = rcs -q -n$(NAME_SYM): -s$(STATE)
CO       = co -q -u -r$(NAME_SYM)
RCSTOUCH = rcstouch

distprep: 
	cd doc ; $(MAKE)

rcsname:
	@files=`perl5 -MExtUtils::Manifest=maniread -MFile::Basename \
	  -e '$$mani = maniread;' \
	  -e '$$, = "\n";' \
	  -e 'print sort grep { -e (dirname ($$_) . "/RCS/" . basename ($$_) . ",v") } keys %$$mani;'` ;\
	for file in $$files ; do \
	  echo "$(RCSNAME) $$file" ; \
	  $(RCSNAME) $$file ; \
	  echo $(CO) $$file ; \
	  $(CO) $$file ; \
	  echo $(RCSTOUCH) $$file ; \
	  $(RCSTOUCH) $$file ; \
	done

# To make a distribution, we copy all files in the manifest with hard
# links.  We then make explicit copies of a few files in order to change
# their mode, create some needed directories needed by the build
# process, and touch the .depend file for the library.  (Eventually, we
# should have .depend files in every directory with C code.  When that's
# done, we should systematically create an empty .depend everywhere it's
# needed.  For now, though, we'll just touch the one file.)

dist: distprep
	mkdir $(RELEASE)
#	tar -cf - -T MANIFEST | (cd $(RELEASE) ; tar -xf -)
	perl5 -MExtUtils::Manifest=maniread,manicopy \
	  -e '$$mani = maniread;' \
	  -e 'manicopy ($$mani, "$(RELEASE)", "best");
	rm -f $(RELEASE)/Makefile $(RELEASE)/Makefile.site
	cp -p Makefile Makefile.site $(RELEASE)
	chmod u+w $(RELEASE)/Makefile $(RELEASE)/Makefile.site
	mkdir $(RELEASE)/bin $(RELEASE)/lib
	find $(RELEASE) -type d -print | xargs chmod 755
	touch $(RELEASE)/source/libsource/.depend
	gtar czf $(ARCHIVE) $(RELEASE)
	rm -rf $(RELEASE)
