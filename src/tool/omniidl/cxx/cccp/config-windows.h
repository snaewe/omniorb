#include <io.h>
#include <malloc.h>

#pragma warning(disable:4786 4250)

#define SIZEOF_UNSIGNED_CHAR 1
#define SIZEOF_INT 4
#define HAVE_STDLIB_H 1
#define HAVE_STRERROR 1
#define DONT_DECLARE_STRERROR 1
#define STDC_HEADERS 1

#pragma warning (disable: 4018)

char *index( char *s, int c );
char *rindex( char *s, int c );
