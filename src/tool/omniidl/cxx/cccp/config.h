#include <omniconfig.h>

#ifdef OMNI_CONFIG_TRADITIONAL

#if defined(__linux__)
#include "config-linux.h"

#elif defined(__sunos__)
#include "config-solaris.h"

#elif defined(__aix__)
#include "config-aix.h"

#elif defined(__WIN32__)
#include "config-windows.h"

#elif defined(__freebsd__)
#include "config-freebsd.h"

#elif defined(__hpux__)
#include "config-hpux.h"

#elif defined(__VMS)
#include "config-vms.h"

#elif defined(__osf1__)
#include "config-osf1.h"

#elif defined(__SINIX__)
#include "config-sinix.h"

#elif defined(__irix__)
#include "config-irix.h"

#elif defined(__nextstep__)
#include "config-nextstep.h"

#elif defined(__darwin__)
#include "config-darwin.h"

#else
#error "You must create a cccp config file for your platform"

#endif

#endif


#define BITS_PER_UNIT SIZEOF_UNSIGNED_CHAR
#define BITS_PER_WORD SIZEOF_INT
#define HOST_BITS_PER_INT SIZEOF_INT

#define TARGET_BELL '\a'
#define TARGET_BS '\b'
#define TARGET_FF '\f'
#define TARGET_NEWLINE '\n'
#define TARGET_CR '\r'
#define TARGET_TAB '\t'
#define TARGET_VT '\v'

#define INCLUDE_DEFAULTS { { 0, 0, 0 } }
#define GCC_INCLUDE_DIR "/usr/include"

#define FATAL_EXIT_CODE 1
#define SUCCESS_EXIT_CODE 0
