#-----------------------------------------------------------------
# C.E.A. IRFU/SIS/LDII
#
# @(#) $Id: CONFIG_SITE_linux.cmd,v 1.3 2011/01/21 13:22:38 cmsmgr Exp $
#
# CONFIG_SITE_linux.cmd
#
# This file contains site dependent configuration commands for linux IOC's. 
# It must be called at the very beginning of the startup script of the IOC.
#
#-----------------------------------------------------------------
# IOC log service
#
# log server IP address
epicsEnvSet("EPICS_IOC_LOG_INET","127.0.0.1 172.16.8.16")
#
# TCP/IP port used by the log server
epicsEnvSet("EPICS_IOC_LOG_PORT","6500")
#
#-----------------------------------------------------------------
# Autosave
#
# Specify where save files should be
epicsEnvSet("AS_DATA","/home/ifmif/data/autosave")
#
#-----------------------------------------------------------------
# Boot time
#
# Specify where boot time files should be
epicsEnvSet("BOOT_DATA","/home/ifmif/data/boot")
#
# End of CONFIG_SITE_linux.cmd
#-----------------------------------------------------------------
