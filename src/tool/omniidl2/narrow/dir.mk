
CXXSRCS       = \
		nr_narrow.cc

OBJS   = \
		nr_narrow.o

DIR_CPPFLAGS = -I../include $(patsubst %,-I%/../include,$(VPATH))

ifdef NTArchitecture
# Note the use of -W0 in CXXOPTIONS - this is used to stop pragma warning 
# messages - should replace with a warning(disable,4068) pragma in the source
# code
DIR_CPPFLAGS += -D "NDEBUG" -D "_WINDOWS" -D "_X86_" -D "NTArchitecture" 
CXXOPTIONS = -ML -W0 -GX -O2 
endif

lib = $(patsubst %,$(LibPattern),narrow)

all:: $(lib)

$(lib): $(OBJS)
	@$(StaticLinkLibrary)
