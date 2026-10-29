# C.E.A. IRFU/SIS/LDISC
#
# @(#) $Id: Makefile 663 2015-11-10 13:52:37Z gomes $
#
# Makefile
#

TOP=../..
include $(TOP)/configure/CONFIG

#=======================================================
# Install CSS/BOY files in <top>/css

CSSS += Status.opi
CSSS += injectorPPS_MPS.opi
CSSS += injectorRF_Beam-Plasma.opi
CSSS += injector_Help_PLC.opi
CSSS += injector_RF_Help12.opi
CSSS += injector_Help_RF_Beam_Plasma.opi
include $(TOP)/configure/RULES

#=======================================================
# Install css subdirectories

SUBDIRS = Pictures Scripts

install:
	@for dir in $(SUBDIRS) ; do \
	if [ -d $$dir ] ; then \
	mkdir -p $(TOP)/css/$$dir ; \
	$(MAKE) -C $$dir ; \
	fi \
	done

clean::
	@for dir in $(SUBDIRS) ; do \
	if [ -d $$dir ] ; then \
	$(MAKE) -C $$dir clean ; \
	fi \
	done


