# Edit COS_INTERFACES to select which COS idl to build into the stub library.
#
# All the interfaces:
#COS_INTERFACES = $(COS_ALL_INT)
# Or just the Notification interfaces:
COS_INTERFACES = $(TIMEBASE_INT) $(COS_EVT_INT) $(COS_NFY_INT) $(COS_LCB_INT)
# Or pick your own subset
# COS_INTERFACES = <pick you own subset from below>


DIR_IDLFLAGS  += -Wbuse_quotes

# omniORB now properly supports long long, so we don't need to turn it off...
#DIR_IDLFLAGS  += -DNOLONGLONG

DIR_CPPFLAGS = $(CORBA_CPPFLAGS) -D_COS_LIBRARY

%.DynSK.cc %SK.cc: %.idl
	$(CORBA_IDL) $(DIR_IDLFLAGS) $^	

.PRECIOUS: %.DynSK.cc %SK.cc
