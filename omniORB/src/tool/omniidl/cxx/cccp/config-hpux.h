#ifdef __hpux__

#define SIZEOF_UNSIGNED_CHAR 1
#define SIZEOF_INT 4
#define HAVE_STDLIB_H 1
#define HAVE_STRERROR 1

#if defined(__HP_cc)
#  define HAVE_ALLOCA 1
#  include <alloca.h>
#endif

#endif
