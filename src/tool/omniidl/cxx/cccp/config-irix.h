/* WARNING!  This file is untested! */

/* Hopefully, SIZEOF_INT 4 will still work on 64 bit IRIX. */

#ifdef __mips__

#define SIZEOF_UNSIGNED_CHAR 1
#define SIZEOF_INT 4
#define HAVE_STDLIB_H 1
#define HAVE_STRERROR 1

#endif
