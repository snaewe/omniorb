#include <omniconfig.h>

#ifdef OMNI_CONFIG_TRADITIONAL

#  if defined(__linux__)
#    include "config-linux.h"

#  elif defined(__sunos__)
#    include "config-solaris.h"

#  elif defined(__aix__)
#    include "config-aix.h"

#  elif defined(__WIN32__)
#    include "config-windows.h"

#  elif defined(__freebsd__)
#    include "config-freebsd.h"

#  elif defined(__hpux__)
#    include "config-hpux.h"

#  elif defined(__VMS)
#    include "config-vms.h"

#  elif defined(__osf1__)
#    include "config-osf1.h"

#  elif defined(__SINIX__)
#    include "config-sinix.h"

#  elif defined(__irix__)
#    include "config-irix.h"

#  elif defined(__nextstep__)
#    include "config-nextstep.h"

#  elif defined(__darwin__)
#    include "config-darwin.h"

#  elif defined(__Lynx__) || defined(__lynxos__)
#    include "config-lynxos.h"

#  else
#    error "You must create a cccp config file for your platform"

#  endif

#else

/* autoconf based build */

/* On some platforms, we do not have a working alloca at all. In that
   case, we just #define alloca to be xmalloc, meaning memory
   allocated with alloca is never freed. That should not be a problem
   because cccp never allocates much memory anyway, and the OS will
   clear up when it exits.

   Note that on Linux platforms, glibc's stdlib.h includes alloca.h,
   which overrides our #define of alloca. That doesn't matter because
   gcc always correctly supports alloca.
*/

#  ifdef OMNIORB_DISABLE_ALLOCA
#    ifdef alloca
#      undef alloca
#    endif
#    define alloca xmalloc
#    ifndef HAVE_ALLOCA
#      define HAVE_ALLOCA 1  /* to prevent alloca.c trying to build one */
#    endif
#  else
#    ifdef HAVE_ALLOCA_H
#      include <alloca.h>
#    endif
#  endif
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
