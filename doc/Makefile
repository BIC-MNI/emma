#
# Makefile for EMMA documentation.
#

#
# Set up the targets
#

default: all

all: emma.ps leastsq.ps rcbf.ps simul.ps Makefile


#
# How to build the general EMMA documentation.
#

emma.dvi: emma.tex emma-help.tex
	latex emma.tex

emma.ps: emma.dvi
	dvips -o emma.ps emma.dvi

#
# How to build the least squares documentation.
#

leastsq.dvi: leastsq.tex
	latex leastsq.tex

leastsq.ps: leastsq.dvi
	dvips -o leastsq.ps leastsq.dvi

#
# How to build the rcbf documentation.
#

rcbf.dvi: rcbf.tex flow.ps structure.ps rcbf-help.tex
	latex rcbf.tex

rcbf.ps: rcbf.dvi flow.ps structure.ps
	dvips -o rcbf.ps rcbf.dvi


#
# How to build the simulation documentation.
#

simul.dvi: simul.tex
	latex simul.tex

simul.ps: simul.dvi
	dvips -o simul.ps simul.dvi
