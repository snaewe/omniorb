/*
 * The functions index() and rindex() are required under WIN32.
 *
 * index() returns a pointer to the first occurrence of charac-
 * ter  c  in  string  s, and rindex() returns a pointer to the
 * last occurrence of character c in string  s.   Both  index()
 * and  rindex()  return  a null pointer if c does not occur in
 * the string.  The null character terminating a string is con-
 * sidered to be part of the string.
 */

#include <string.h>


char *index( char *s, int c )
{
  if( s != NULL ) {
    while( *s != '\0' && *s != c ) s++;

    if( *s == '\0' && c != '\0' )
      s = NULL;
  }
  return s;
}


char *rindex( char *s, int c )
{
  char *ss;

  if( s != NULL ) {
    ss = s; 
    while( *s != '\0' ) s++;
    do {
      if( *s == c )
	return s;
    } while( s-- != ss );
  }
  return NULL;
}
