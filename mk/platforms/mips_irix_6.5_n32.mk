#
# mips_irix_6.5_n32.mk - make variables and rules specific to SGI Irix 6.5
# 			 with the MIPSpro 7.2.1 compilers
#

# The ORB has been verified to work on this platform with MIPSpro 7.2.1
# compiler & runtime. Patches applied:
#I  patchSG0002992       10/12/98  Patch SG0002992: MIPSpro 7.2.1 Compiler 
#                                                   Back-end #1
#I  patchSG0003048       10/12/98  Patch SG0003048: MIPSpro 7.2.1 C++
#                                                   front-end rollup #1
#I  patchSG0003077       10/12/98  Patch SG0003077: MIPSpro 7.2.1 C
#                                                   front-end rollup #1
#I  patchSG0003131       08/25/98  Patch SG0003131: 7.2.1 base compiler
#                        runtime environment for IRIX 6.2/6.3/6.4/6.5
#I  patchSG0003139       08/25/98  Patch SG0003139: libmp rollup
#

IRIX_n32 = 1

#
# Include common IRIX 6.4, 6.5 things
#

include $(THIS_IMPORT_TREE)/mk/platforms/mips_irix_6.4_6.5_common.mk

