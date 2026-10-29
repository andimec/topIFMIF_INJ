# C.E.A. IRFU/SIS/LDISC
#
# @(#) $Id: Makefile 1569 2012-09-07 14:15:51Z lussignol $
#
# Makefile
#

TOP=../../..
include $(TOP)/configure/CONFIG
include $(TOP)/configure/RULES

#=======================================================
# Install scripts in <top>/css/Scripts

#FILES += script.sh
FILES += injector-params.sh

DESTDIR = $(TOP)/css/$(notdir $(CURDIR))

install:
	@for file in $(FILES) ; do \
	if [ -f $$file ] ; then \
	echo "installing $$file in $(DESTDIR)" ; \
	cp -f $$file $(DESTDIR) ; \
	fi \
	done

