# C.E.A. IRFU/SIS/LDISC
#
# @(#) $Id: Makefile 660 2015-11-10 13:47:45Z gomes $
#
# Makefile
#

TOP=../..
include $(TOP)/configure/CONFIG

#=======================================================
# Install CSS/BOY files in <top>/css

CSSS += Automatic_Action.opi
# CSSS += Four_Grid_Analyser.opi
CSSS += Injector_FGA.opi
CSSS += Injector_Calorimetry.opi
CSSS += Injector_Main.opi
CSSS += Injector_Launcher.opi
#CSSS += Help_Vacuum.opi
CSSS += Injector_Help_Vacuum.opi
CSSS += Help_Status_RF.opi
#CSSS += Help_Status.opi
CSSS += Injector_Help_Status.opi
#CSSS += Help_Calorimetry.opi
CSSS += Injector_Help_Calorimetry.opi
CSSS += Injector_Timing.opi
#CSSS += Injector_Vacuum_Saclay.opi
CSSS += Injector_Vacuum.opi
#CSSS += MKS_HD.opi
CSSS += Injector_MKS_HD.opi
CSSS += injector_water_skid.opi
CSSS += Injector_magnets_status.opi
#CSSS += RF_Modbus.opi
CSSS += Injector_RF_Modbus.opi
CSSS += injectorHVPS_interlocks.opi
CSSS += injectorIEPS_interlocks.opi





include $(TOP)/configure/RULES

#=======================================================
# Install css subdirectories

SUBDIRS = Pictures Scripts CHOPPER_IM_OPI

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


