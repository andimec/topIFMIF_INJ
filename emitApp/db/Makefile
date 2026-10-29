# C.E.A. IRFU/SIS/LDII
#
# @(#) $Id: Makefile,v 1.5 2012/08/08 13:07:45 cmsmgr Exp $
#
# Makefile
#

TOP=../..
include $(TOP)/configure/CONFIG

#=======================================================
# Install database record or stream protocol files in <top>/db

#DB += <name>.db
#DB += <name>_proto.db
#DB += ...
DB += emitProcess.db
DB += emitIcv150.db
DB += emitIcv196.db
DB += emitIcv714.db
DB += emitMotor.db
DB += emitServo.db
DB += emitServoProto.db
DB += emitVxStats.db

DB += injectorFC_Calorimetry.db
DB += injectorIC_Calorimetry.db
DB += injectorGE_temp.db
DB += injectorHV_Spark.db

include $(TOP)/configure/RULES
