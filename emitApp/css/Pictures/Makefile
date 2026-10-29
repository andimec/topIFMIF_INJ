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
# Install pictures in <top>/css/Pictures

#FILES += picture.jpg
FILES += css1.png 
FILES += css2.png 
FILES += emitIfmif.png  
FILES += epics.png
FILES += ifmifInj.png
FILES += logoCea.png
FILES += logoIrfu.png

DESTDIR = $(TOP)/css/$(notdir $(CURDIR))

install:
	@for file in $(FILES) ; do \
	if [ -f $$file ] ; then \
	echo "installing $$file in $(DESTDIR)" ; \
	cp -f $$file $(DESTDIR) ; \
	fi \
	done

