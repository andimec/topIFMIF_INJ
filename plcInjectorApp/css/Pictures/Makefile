# C.E.A. IRFU/SIS/LDISC
#
# @(#) $Id: Makefile 663 2015-11-10 13:52:37Z gomes $
#
# Makefile
#

TOP=../../..
include $(TOP)/configure/CONFIG
include $(TOP)/configure/RULES

#=======================================================
# Install pictures in <top>/css/Pictures

#FILES += picture.jpg

DESTDIR = $(TOP)/css/$(notdir $(CURDIR))

install:
	@for file in $(FILES) ; do \
	if [ -f $$file ] ; then \
	echo "installing $$file in $(DESTDIR)" ; \
	cp -f $$file $(DESTDIR) ; \
	fi \
	done

