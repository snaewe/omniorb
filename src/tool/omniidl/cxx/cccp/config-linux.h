#if defined(__x86__) || defined(__powerpc__) || defined(__s390__)

#define SIZEOF_UNSIGNED_CHAR 1
#define SIZEOF_INT 4
#define HAVE_STDLIB_H 1
#define HAVE_STRERROR 1
#define HAVE_VPRINTF 1

#else
#error "You must set definitions for your architecture in config-linux.h"

#endif
