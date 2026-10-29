# C.E.A. IRFU/SIS/LDISC
#
# $Id$
#
# Makefile at top of application tree
#

TOP = .
include $(TOP)/configure/CONFIG

# Make directory configure first
DIRS := $(DIRS) $(filter-out $(DIRS), configure)

# Make applications explicitly
# This may be necessary if dependencies exist between applications
# and you want to explicit the build order
#DIRS := $(DIRS) $(filter-out $(DIRS), xxxApp)

ifneq ($(shell ls Icv108App 2>/dev/null),)
DIRS := $(DIRS) $(filter-out $(DIRS), Icv108App)
endif

ifneq ($(shell ls listRecApp 2>/dev/null),)
DIRS := $(DIRS) $(filter-out $(DIRS), listRecApp)
endif

# Make other applications alphabetically
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *App))

# Make IOC's
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard iocBoot))

include $(TOP)/configure/RULES_TOP
