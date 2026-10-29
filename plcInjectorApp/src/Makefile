# C.E.A. IRFU/SIS/LDII
#
# @(#) $Id: Makefile 663 2015-11-10 13:52:37Z gomes $
#
# Makefile
#

TOP=../..
include $(TOP)/configure/CONFIG

#=======================================================
# Build the IOC support library plcInjector
# and install it in <top>/lib

#USR_CFLAGS += -fno-strict-aliasing

# build the library for Linux and vxWorks
#LIBRARY_IOC += plcInjector
# or for Linux only
#LIBRARY_IOC_Linux += plcInjector
# or for vxWorks only
#LIBRARY_IOC_vxWorks += plcInjector

# Source files
#plcInjector_SRCS += <name>.c
#plcInjector_SRCS += ...

# SNL programs
# Make the run-time code reentrant, thus allowing more 
# than one instance of the state program to run on the IOC
#<name>_SNCFLAGS = +r
#plcInjector_SRCS += <name>.st

#=======================================================
# Install plcInjectorSupport.dbd and plcInjectorInclude.dbd in <top>/dbd

DBD += plcInjectorSupport.dbd
DBD += plcInjectorInclude.dbd


#=======================================================
# Build and install plcInjector.dbd in <top>/dbd
# plcInjector.dbd derives from plcInjectorInclude.dbd

DBD += plcInjector.dbd

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

#PROD_HOST += <name>

# option +m generates a C main program
#<name>_SNCFLAGS = +m
#<name>_SRCS += <name>.st
#<name>_LIBS += seq pv
#<name>_LIBS += $(EPICS_BASE_HOST_LIBS)

#=======================================================
# Build vxWorks object <name>.o from file <name>.c
# and install it in <top>/bin

#OBJS_vxWorks += <name>

#=======================================================
# Perl, csh, tcl, awk etc. script installation for Linux

#SCRIPTS_HOST += <name>

include $(TOP)/configure/RULES

#=======================================================
# Make man pages and html documentation with doxygen

install: doxygen
doxygen:
	@if [ -f doxygen.cfg ] ; then doxygen doxygen.cfg ; fi

