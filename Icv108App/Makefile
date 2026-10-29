# C.E.A. IRFU/SIS/LDII
#
# @(#) $Id: Makefile 2360 2014-04-15 12:18:41Z lussignol $
#
# Makefile
#

TOP = ..
include $(TOP)/configure/CONFIG

DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *src*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *Src*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *db*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *Db*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *gui*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *Gui*))

include $(TOP)/configure/RULES_DIRS

clean:
	@echo "deleting html & man"
	@$(RMDIR) html man 
