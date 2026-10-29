# C.E.A. IRFU/SIS/LDII
#
# @(#) $Id: Makefile,v 1.5 2012/08/08 13:07:45 cmsmgr Exp $
#
# Makefile
#

TOP=../..
include $(TOP)/configure/CONFIG

#=======================================================
# Build the IOC support library emit
# and install it in <top>/lib

#USR_CFLAGS += -fno-strict-aliasing

# build the library for Linux and vxWorks
#LIBRARY_IOC += emit
# or for Linux only
#LIBRARY_IOC_Linux += emit
# or for vxWorks only
LIBRARY_IOC_vxWorks += emit

# Source files
#emit_SRCS += <name>.c
#emit_SRCS += ...

# SNL programs
# Make the run-time code reentrant, thus allowing more 
# than one instance of the state program to run on the IOC
#<name>_SNCFLAGS = +r
#emit_SRCS += <name>.st
emit_SNCFLAGS = +r
emit_SRCS += emitSnlAlims.st
emit_SRCS += emitSnlMotor.st

#=======================================================
# Install emitSupport.dbd and emitInclude.dbd in <top>/dbd

DBD += emitSupport.dbd
DBD += emitInclude.dbd

#=======================================================
# Build and install emit.dbd in <top>/dbd
# emit.dbd derives from emitInclude.dbd

DBD += emit.dbd

#=======================================================
# Build a SNL program as a Linux standalone application
# and install it in <top>/bin.
#
# To run this application, enter the following command:
#   <name> "param1=value1,param2=value2,..."
#
# The command option -s starts an IOC shell with the application,
# thus allowing to issue sequence commands like seqShow and exit:
#   <name> -s "param1=value1,param2=value2,..."

PROD_HOST += emitSnlEmu_v2

# option +m generates a C main program
#<name>_SNCFLAGS = +m
#<name>_SRCS += <name>.st
#<name>_LIBS += seq pv
#<name>_LIBS += $(EPICS_BASE_HOST_LIBS)
emitSnlEmu_v2_SNCFLAGS = +m
# Patricia commented this
# emitSnlEmu_v1_SRCS += emitSnlEmu_v1.st # original SNL
emitSnlEmu_v2_SRCS += emitSnlEmu_v2.st # SNL optm emittance (no scan)
emitSnlEmu_v2_LIBS += seq pv
emitSnlEmu_v2_LIBS += $(EPICS_BASE_HOST_LIBS)

#=======================================================
# Build vxWorks object <name>.o from file <name>.c
# and install it in <top>/bin

#OBJS_vxWorks += <name>

#=======================================================
# Perl, csh, tcl, awk etc. script installation for Linux

#SCRIPTS_HOST += <name>
#SCRIPTS_HOST += emitShell_v1.sh # Patricia: this is for previous version of SNL in the OPI
SCRIPTS_HOST += emitShell_v2.sh
SCRIPTS_HOST += emitIocShell.sh
SCRIPTS_HOST += emitServoShell.sh
#SCRIPTS_HOST += emitUpper.sh
SCRIPTS_HOST += emitCssLauncher.sh

include $(TOP)/configure/RULES

#=======================================================
# Make man pages and html documentation with doxygen

install: doxygen
doxygen:
	@if [ -f doxygen.cfg ] ; then doxygen doxygen.cfg ; fi

