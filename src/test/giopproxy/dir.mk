CXXSRCS = server.cc rewrite_ior.cc test_rewrite_ior.cc

DIR_CPPFLAGS = $(CORBA_CPPFLAGS) 

# Add this -I flag to include the header file giopObjectInfo.h from the
# orbcore. This is an ORB internal header but we need it for rewrite_ior.cc
#
DIR_CPPFLAGS += $(patsubst %,-I%/../../lib/omniORB2/orbcore,$(VPATH))

server = $(patsubst %,$(BinPattern),server)
test_rewrite_ior = $(patsubst %,$(BinPattern),test_rewrite_ior)

all:: $(server) $(test_rewrite_ior)

clean::
	$(RM) $(server) $(test_rewrite_ior)

$(server): server.o $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))

$(test_rewrite_ior): test_rewrite_ior.o rewrite_ior.o $(CORBA_LIB_DEPEND)
	@(libs="$(CORBA_LIB)"; $(CXXExecutable))
