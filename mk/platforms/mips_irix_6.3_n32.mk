#
# mips_irix_6.3_n32.mk - make variables and rules specific to SGI Irix 6.3
# 			 with the MIPSpro 7.2.0 compiler
#

# The ORB has been verified to work on this platform with MIPSpro 7.2.0
# compiler & runtime. Patches applied:
#
# I  patchSG0001645       09/14/98  Patch SG0001645: IRIX 6.2 & 6.3 POSIX header file updates
# I  patchSG0001759       08/06/97  Patch SG0001759: 'Browse OutBox Page'/ns_fasttrack error
# I  patchSG0001860       06/16/98  Patch SG0001860: IRIX 6.3 PPP bug fixes
# I  patchSG0001894       06/16/98  Patch SG0001894: Server-based X Num Lock Support
# I  patchSG0001959       06/16/98  Patch SG0001959: NFS over TCP server support for 6.3
# I  patchSG0001974       06/16/98  Patch SG0001974: gmemusage security hole
# I  patchSG0001995       06/16/98  Patch SG0001995: mount_dos 'pwd'/'mv' fix
# I  patchSG0002009       06/16/98  Patch SG0002009: IGLOO (IrisGL On OpenGL) Rollup for 6.2 and 6.3
# I  patchSG0002023       06/16/98  Patch SG0002023: Fam fix for netwr_client
# I  patchSG0002037       06/16/98  Patch SG0002037: Motif libXm and libSgm fixes for Irix 6.3
# I  patchSG0002077       06/16/98  Patch SG0002077: runpriv security hole
# I  patchSG0002087       06/16/98  Patch SG0002087: libc bug fixes and enhancements
# I  patchSG0002090       06/16/98  Patch SG0002090: xlock security fix
# I  patchSG0002112       06/16/98  Patch SG0002112: 6.3 PCI Audio Option support + general audio and MIDI synthesizer bug-fixes
# I  patchSG0002133       06/16/98  Patch SG0002133: talkd security
# I  patchSG0002144       06/16/98  Patch SG0002144: Mount command rollup for IRIX 6.3
# I  patchSG0002148       06/16/98  Patch SG0002148: Automount command rollup for IRIX 6.3
# I  patchSG0002153       06/16/98  Patch SG0002153: Multiple fixes for X libraries
# I  patchSG0002161       09/14/98  Patch SG0002161: Pthread library fixes
# I  patchSG0002180       06/16/98  Patch SG0002180: IRIX 6.3 Networking Commands
# I  patchSG0002213       06/16/98  Patch SG0002213: IRIX 6.2/6.3/6.4 ordist Security
# I  patchSG0002232       06/16/98  Patch SG0002232: commands security fixes
# I  patchSG0002240       06/16/98  Patch SG0002240: security fix for syserr
# I  patchSG0002244       06/16/98  Patch SG0002244: 6.3 security fix for cdplayer
# I  patchSG0002257       06/16/98  Patch SG0002257: ONC3 6.3 AutoFS Rollup #2
# I  patchSG0002275       06/16/98  Patch SG0002275: IRIX 6.3 Digital Media bug fixes
# I  patchSG0002279       06/16/98  Patch SG0002279: Status monitor fixes for 6.3
# I  patchSG0002288       06/16/98  Patch SG0002288: XFS rollup #3 for 6.3
# I  patchSG0002334       06/16/98  Patch SG0002334: xfsdump/xfsrestore fixes for 6.3-O2-10K
# I  patchSG0002366       06/16/98  Patch SG0002366: 6.3 NFS Kernel Rollup #3
# I  patchSG0002387       06/16/98  Patch SG0002387: O2 graphics bugfix and performance fixes
# I  patchSG0002392       06/16/98  Patch SG0002392: Korean MBWC locale alias/codeset information in iconv spec.
# I  patchSG0002413       06/16/98  Patch SG0002413: IRIX 6.3,6.4 Routing Enhancements
# I  patchSG0002447       06/16/98  Patch SG0002447: X server rollup
# I  patchSG0002485       06/16/98  Patch SG0002485: 6.3 O2 Video w/passthru fix
# I  patchSG0002537       06/16/98  Patch SG0002537: Desktop rollup #2 for 6.3
# I  patchSG0002603       06/16/98  Patch SG0002603: O2 ethernet driver fixes
# I  patchSG0002652       06/16/98  Patch SG0002652: Partial workaround for spurious mouse events
# I  patchSG0002687       11/02/98  Patch SG0002687: MIPSpro 7.2 Compiler Back-end #2
# I  patchSG0002704       06/16/98  Patch SG0002704: Fix for kernel panic in shmiq_rput
# I  patchSG0002747       06/16/98  Patch SG0002747: IRIX 6.3 Networking Kernel Rollup #4
# I  patchSG0002752       11/02/98  Patch SG0002752: 7.2 compiler runtime environment for IRIX 6.2/6.3/6.4
# I  patchSG0002753       06/16/98  Patch SG0002753 OutBox security fixes
# I  patchSG0002757       06/16/98  Patch SG0002757: O2 adaptec rollup
# I  patchSG0002774       06/16/98  Patch SG0002774: O2 PROM rollup
# I  patchSG0002796       06/16/98  Patch SG0002796: fix for O2 tserialio and serial drivers
# I  patchSG0002801       06/16/98  Patch SG0002801: IRIX 6.3 kernel rollup
# I  patchSG0002838       06/16/98  Patch SG0002838 6.3 tape driver fixes and support for additional tape drives
# I  patchSG0002933       06/16/98  Patch SG0002933: O2 IDE for big memory and 250Mhz cpu

IRIX_n32 = 1

#
# Include common IRIX 6.3, 6.4, 6.5 things
#

include $(THIS_IMPORT_TREE)/mk/platforms/mips_irix_6.4_6.5_common.mk

