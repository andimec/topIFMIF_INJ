# C.E.A. IRFU/SIS/LDISC
#
# @(#) $Id: Makefile 1703 2013-01-17 09:07:34Z lussignol $
#
# Makefile
#

TOP=../..
include $(TOP)/configure/CONFIG

#=======================================================
# Install CSS/BOY files in <top>/css

#CSSS += file.opi
CSSS += emuLauncher.opi
CSSS += emitMeasurement_v2.opi #og: CSSS += emitMeasurement_v1.opi
CSSS += emuHelp.opi
CSSS += emuMotor.opi
CSSS += emuHelpMotor.opi
CSSS += emuMotorUser_v2.opi #og: CSSS += emuMotorUser_v1.opi
CSSS += emuAlims.opi
CSSS += HVSparkSuppressor.opi

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


