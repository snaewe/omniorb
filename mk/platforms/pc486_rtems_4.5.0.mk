###########################################################################
#
# $Header$
#
# Copyright (c) 1999-2000 ConnectTel, Inc. All Rights Reserved.
#  
# MODULE DESCRIPTION: pc486_rtems_4.5.0.mk 
# 
# Define variables and rules specific to OARcorp's RTEMS. This file is 
# BSP dependent. It should be modified to match the requirements of a 
# particular BSP.
#
#  by: Rosimildo da Silva:
#      rdasilva@connecttel.com
#      http://www.connecttel.com
#
# MODIFICATION/HISTORY:
#
# $Log$
# Revision 1.4.2.1  2003/03/23 21:03:28  dgrisby
# Start of omniORB 4.1.x development branch.
#
# Revision 1.2.2.1  2000/08/21 13:31:49  sll
# Merged make rules update from omni3_develop
#
# Revision 1.1.4.2  2000/08/21 11:28:59  sll
# *** empty log message ***
#
# Revision 1.1.2.1  2000/08/17 15:37:51  sll
# Merged RTEMS port.
#
#
###########################################################################

#
# make variables and rules specific to OARcorp's RTEMS
# This file is BSP dependent. It should be modified to match the requirements
# of a particular BSP.
#

include $(THIS_IMPORT_TREE)/mk/rtems.mk

#
# Default location of the omniORB2 configuration file.
#
CONFIG_DEFAULT_LOCATION = /etc/omniorb.cfg

#
# Rules to create the executables for the BSP ==> PC386 
#
# This is necessary because RTEMS does not have a "real main()", and
# the entry point for the user, it is the init task. The init task,
# provided in the module rtems_init, does all network, and file system
# initialization, before calling the "cc_main()" user entry point.
IMPORT_CPPFLAGS += -D__x86__  -Dmain=cc_main

#
# NOTE: This module has RTEMS configuration, and it needs to be
#       modified to match your target requirements, specially
#       all network settings.
#
RTEMS_MAIN_MODULE = $(TOP)/src/examples/rtems/rtems_init.cc


RELOCADDR       = 0x00100000
START16FILE     = $(PROJECT_RELEASE)/lib/start16.bin
START16ADDR     = 0x00097C00
HEADERADDR      = 0x00097E00
define CXXExecutable
(set -x; \
 $(RM) $@.exe; \
 $(CXXLINK) -o $@.exe $(CXXLINKOPTIONS) -Wl,-Ttext,$(RELOCADDR) \
 $(IMPORT_LIBRARY_FLAGS) $(CXXFLAGS) $(RTEMS_MAIN_MODULE) \
    $(filter-out $(LibSuffixPattern),$^) $$libs; \
 $(OBJCOPY) -O binary $@.exe $@.bin; \
 $(PROJECT_TOOLS)/bin2boot -v $@.bt $(HEADERADDR) \
   $(START16FILE) $(START16ADDR) 0 $@.bin $(RELOCADDR) 0; \
 $(NM) -g -n $@.exe > $@.num; \
)
endef

define CExecutable
(set -x; \
 $(RM) $@.exe; \
 $(CLINK) $(RTEMS_MAIN_MODULE) -o $@.exe $(CLINKOPTIONS) -Wl,-Ttext,$(RELOCADDR) \
 $(IMPORT_LIBRARY_FLAGS) $(CXXFLAGS) \
    $(filter-out $(LibSuffixPattern),$^) $$libs; \
 $(OBJCOPY) -O binary $@.exe $@.bin; \
 $(PROJECT_TOOLS)/bin2boot -v $@.bt $(HEADERADDR) \
   $(START16FILE) $(START16ADDR) 0 $@.bin $(RELOCADDR) 0; \
 $(NM) -g -n $@.exe > $@.num; \
)
endef

