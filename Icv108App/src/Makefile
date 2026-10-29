# C.E.A. IRFU/SIS/LDII
#
# @(#) $Id: Makefile 2360 2014-04-15 12:18:41Z lussignol $
#
# Makefile
#

TOP=../..
include $(TOP)/configure/CONFIG

#=======================================================
# Build the IOC support library Icv108
# and install it in <top>/lib

USR_CFLAGS += -O0
# build the library for Linux and vxWorks
#LIBRARY_IOC += Icv108
# or for Linux only
#LIBRARY_IOC_Linux += Icv108
# or for vxWorks only
LIBRARY_IOC_vxWorks += Icv108
#LIBRARY_IOC_vxWorks += Symb
# Source files
Icv108_SRCS += Icv108.c
#Icv108_SRCS += ...

# SNL programs
# Make the run-time code reentrant, thus allowing more 
# than one instance of the state program to run on the IOC
#<name>_SNCFLAGS = +r
#Icv108_SRCS += sncIcv108_Test.stt

#=======================================================
# Install Icv108Support.dbd and Icv108Include.dbd in <top>/dbd

DBD += Icv108Support.dbd
DBD += Icv108Include.dbd

#=======================================================
# Build and install Icv108.dbd in <top>/dbd
# Icv108.dbd derives from Icv108Include.dbd

DBD += Icv108.dbd

#=======================================================
# Build a SNL program as a Linux standalone application
# and install it in <top>/bin

#PROD_HOST += <name>
#<name>_SNCFLAGS = +m
#<name>_SRCS += <name>.st
#<name>_LIBS += seq pv
#<name>_LIBS += $(EPICS_BASE_HOST_LIBS)

#=======================================================
# Build vxWorks object <name>.o from file <name>.c
# and install it in <top>/bin

#OBJS_vxWorks += <name>

include $(TOP)/configure/RULES

install: doxygen
doxygen:
	@if [ -f doxygen.cfg ] ; then doxygen doxygen.cfg ; fi