# C.E.A. IRFU/SIS/LDII
#
# @(#) $Id: Makefile,v 1.3 2011/01/21 13:22:38 cmsmgr Exp $
#
# Makefile
#

TOP=..

include $(TOP)/configure/CONFIG

# CHECK_RELEASE controls the consistency checking of the support
# applications defined in the $(TOP)/configure/RELEASE* files.
# Normally CHECK_RELEASE should be set to YES.
# Set CHECK_RELEASE to NO to disable checking completely.
# Set CHECK_RELEASE to WARN to perform consistency checking,
# but continue the build even if conflicts are found.
CHECK_RELEASE = YES

TARGETS = $(CONFIG_TARGETS)
CONFIGS += $(subst ../,,$(wildcard $(CONFIG_INSTALLS)))

include $(TOP)/configure/RULES

# The following target allow to replace the tags contained int the startup scripts
# CONFIG_SITE_linux.cmd and CONFIG_SITE_vxWorks.cmd the first time the make is done:
# - _HOSTNAME_ and _HOSTIP_ are replaced by the host name and the ip address
#   of the machine on which the make is done.
# - _USER_ _UID_ and _GID_ are replaced by the name the id and the group of the user
#   who is doing the make.
# - _IFMIFUID_ and _IFMIFGID_ are replaced by the id and the group of the user ifmif.
# For an alternate installation, comment these targets and change the tags
# manually in the files.

install: tags

tags:
	@if [ -f CONFIG_SITE_linux.cmd ] ; then \
	echo "setting host $(shell hostname) $(shell hostname -i)" ; \
	awk '{ gsub(/_HOSTIP_/, "$(shell hostname -i)") ; print }' CONFIG_SITE_linux.cmd > temp ; \
	cp -f temp CONFIG_SITE_linux.cmd ; \
	awk '{ gsub(/_HOSTIP_/, "$(shell hostname -i)") ; print }' CONFIG_SITE_vxWorks.cmd > temp ; \
	cp -f temp CONFIG_SITE_vxWorks.cmd ; \
	awk '{ gsub(/_HOSTNAME_/, "$(shell hostname)") ; print }' CONFIG_SITE_vxWorks.cmd > temp ; \
	cp -f temp CONFIG_SITE_vxWorks.cmd ; \
	echo "setting user ifmif uid/gid" ; \
	awk '{ gsub(/_IFMIFUID_/, "$(shell id -u ifmif)") ; print }' CONFIG_SITE_vxWorks.cmd > temp ; \
	cp -f temp CONFIG_SITE_vxWorks.cmd ; \
	awk '{ gsub(/_IFMIFGID_/, "$(shell id -g ifmif)") ; print }' CONFIG_SITE_vxWorks.cmd > temp ; \
	cp -f temp CONFIG_SITE_vxWorks.cmd ; \
	echo "setting user name $(shell id -nu) uid $(shell id -u) gid $(shell id -g)" ; \
	awk '{ gsub(/_USER_/, "$(shell id -nu)") ; print }' CONFIG_SITE_vxWorks.cmd > temp ; \
	cp -f temp CONFIG_SITE_vxWorks.cmd ; \
	awk '{ gsub(/_UID_/, "$(shell id -u)") ; print }' CONFIG_SITE_vxWorks.cmd > temp ; \
	cp -f temp CONFIG_SITE_vxWorks.cmd ; \
	awk '{ gsub(/_GID_/, "$(shell id -g)") ; print }' CONFIG_SITE_vxWorks.cmd > temp ; \
	cp -f temp CONFIG_SITE_vxWorks.cmd ; \
	rm -f temp ; \
	fi

