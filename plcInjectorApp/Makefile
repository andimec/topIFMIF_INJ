# C.E.A. IRFU/SIS/LDII
#
# @(#) $Id: Makefile 663 2015-11-10 13:52:37Z gomes $
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
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *css*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *Css*))

include $(TOP)/configure/RULES_DIRS

clean:
	@echo "deleting html & man"
	@$(RMDIR) html man 
