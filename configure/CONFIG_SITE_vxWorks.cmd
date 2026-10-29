#-----------------------------------------------------------------
# C.E.A. IRFU/SIS/LDII
#
# @(#) $Id: CONFIG_SITE_vxWorks.cmd,v 1.3 2011/01/21 13:22:38 cmsmgr Exp $
#
# CONFIG_SITE_vxWorks.cmd
#
# This file contains site dependent configuration commands for vxWorks IOC's. 
# It must be called at the very beginning of the startup script of the IOC.
#
#-----------------------------------------------------------------
# Network
#
# Add a network route
#routec "add -net -netmask 255.255.0.0 132.167.0.0 132.166.59.254"
#
# Show route table
#netstat "-r"
#
#-----------------------------------------------------------------
# NFS
#
# Mount the NFS folders from the LIPAc main NAS (replacement of the old main server)
hostAdd("lipac-main-nas", "172.16.8.23")
#
# Mount the user file system from the LIPAc main NAS
nfsMount("lipac-main-nas","/volume1/ifmifmgr","/home/ifmifmgr")
#
# Mount the data file system from the LIPAc main NAS
nfsMount("lipac-main-nas","/volume1/data/data_INJ","/var/inj-data")
#
# Mount epicsmgr file system from the LIPAc main NAS
nfsMount("lipac-main-nas","/volume1/epicsmgr","/home/epicsmgr")
#
# Set UNIX authentication parameters for user ifmif
nfsAuthUnixSet("lipac-main-nas",1025,100,0,0)
#
# Show mounted file systems
nfsDevShow
#
#-----------------------------------------------------------------
# Time Service
#
# Declare a NTP Server ...
putenv "EPICS_TS_NTP_INET=172.16.8.20"
#
# ... or use the boot host as time server
#putenv "EPICS_TS_NTP_INET="
#
# Define local timezone. The format required is:
#	<name>::<minutesWest>:<start daylight>:<end daylight>
#	where the start and end are mmddhh (month,day,hour)
# PARIS: UTC+1 (-60 minutes west)
# DST for 2013: Mars 31 2h - October 27 3h
#putenv "TIMEZONE=MET::-60:033102:102703"

# Define local timezone. The format required is:
#       <name>::<minutesWest>:<start daylight>:<end daylight>
#       where the start and end are mmddhh (month,day,hour)
# Tokyo: UTC+9 (540 minutes west)
# DST for 2015: Nothing.
putenv "TIMEZONE=JST::-540:000000:000000"



#
#-----------------------------------------------------------------
# IOC log service
#
# log server IP address
putenv "EPICS_IOC_LOG_INET=127.0.0.1 172.16.8.16"
#
# TCP/IP port used by the log server
putenv "EPICS_IOC_LOG_PORT=6500"
#
#-----------------------------------------------------------------
# Autosave
#
# Specify where save files should be
#AS_DATA = "/home/ifmif/data/autosave"
AS_DATA = "/var/inj-data/autosave"
#
#-----------------------------------------------------------------
# Boot time
#
# Specify where boot time files should be
BOOT_DATA = "/home/ifmif/data/boot"
#
#-----------------------------------------------------------------
# Change the backspace character to DEL for telnet/rlogin.
# The default backspace character is CTRL-H (0x08).
#
#tyBackspaceSet 0x7f
#
# End of CONFIG_SITE_vxWorks.cmd
#-----------------------------------------------------------------
