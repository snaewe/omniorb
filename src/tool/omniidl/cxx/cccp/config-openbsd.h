#ifdef __x86__

#define SIZEOF_UNSIGNED_CHAR 1
#define SIZEOF_INT 4
#define HAVE_STDLIB_H 1
#define HAVE_STRERROR 1

#else
#error "You must set definitions for your architecture in config-openbsd.h"

#endif
