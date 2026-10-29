# C.E.A. IRFU/SIS/LDII
#
# @(#) $Id: Makefile 660 2015-11-10 13:47:45Z gomes $
#
# Makefile
#
TOP=../..
include $(TOP)/configure/CONFIG
#====================================================================#
#=====    /home/ifmifmgr/topIFMIF/injectorApp/src/Makefile    =======#
#=====    Daniel BOGARD 07 Juillet 2010                       =======#
#====================================================================#
#
# Build the IOC support library injector
# and install it in <top>/lib
#
# build the library for Linux and vxWorks
#LIBRARY_IOC += injector
# or for Linux only
LIBRARY_IOC_Linux += injector
# or for vxWorks only
#LIBRARY_IOC_vxWorks += injector

# Source files
#injector_SRCS += <name>.c
injector_SRCS += injectorgensubEcrBits.c
injector_SRCS += injectorgensubEcrMots.c
injector_SRCS += injectorgensubLectMots.c

# SNL programs
# Make the run-time code reentrant, thus allowing more 
# than one instance of the state program to run on the IOC
#<name>_SNCFLAGS = +r
#<snl_hton>_SNCFLAGS = +r
#<snl_htoff>_SNCFLAGS = +r
#injector_SRCS += <name>.st

injector_SRCS += injectorMKS_SRC.stt

injector_SRCS += injectorTiming.stt

injector_SRCS += injectorMKS_LEBT.stt

injector_SRCS += injectorRampe_S1.stt

injector_SRCS += injectorRampe_S2.stt

injector_SRCS += injectorHTON.stt

injector_SRCS += injectorHTOFF.stt


injector_SRCS += injectorRampe_C1.stt

injector_SRCS += injectorRampe_C2.stt

injector_SRCS += injectorRF.stt

injector_SRCS += injectorFGA.stt

#=======================================================
# Install injectorSupport.dbd and injectorInclude.dbd in <top>/dbd

DBD += injectorSupport.dbd
DBD += injectorInclude.dbd


#=======================================================
# Build and install injector.dbd in <top>/dbd
# injector.dbd derives from injectorInclude.dbd

DBD += injector.dbd

#=======================================================
# Build a SNL program as a Linux standalone application
# and install it in <top>/bin

#PROD_HOST += <name>
#PROD_HOST += <snl_hton>
#PROD_HOST += <snl_htoff>
#<name>_SNCFLAGS = +m
#<snl_hton>_SNCFLAGS = +m
#<snl_htoff>_SNCFLAGS = +m
#<name>_SRCS += <name>.st
#injector_LIBS += seq pv
#<name>_LIBS += $(EPICS_BASE_HOST_LIBS)
#=======================================================
# Build vxWorks object <name>.o from file <name>.c
# and install it in <top>/bin

#OBJS_vxWorks += <name>

# OBJS_vxWorks +=injectorMKS_SRC
# OBJS_vxWorks +=injectorMKS_LEBT
# OBJS_vxWorks +=injectorTiming

# OBJS_vxWorks +=injectorHTON
# OBJS_vxWorks +=injectorHTOFF
# OBJS_vxWorks +=injectorRampe_C1
# OBJS_vxWorks +=injectorRampe_C2
# OBJS_vxWorks +=injectorRampe_S1
# OBJS_vxWorks +=injectorRampe_S2
# OBJS_vxWorks +=injectorRF
# OBJS_vxWorks +=injectorFGA

include $(TOP)/configure/RULES
