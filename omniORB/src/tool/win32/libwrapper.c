/*
 * libwrapper.c - wrapper around Visual C++'s "LIB" program to perform
 * file name translation (from unix-like to dos-like names).
 *
 * Both GNUWIN32 and OpenNT file names are supported - the first argument
 * to this program specifies which.
 *
 * Compile this program with "cl libwrapper.c advapi32.lib".
 */

#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <string.h>

#include "pathmapping.h"

int main(int argc, char **argv)
{
  int rc;
  int i;

  if (argc > 1 && strcmp(argv[1], "-gnuwin32") == 0) {
    GetMounts(1);
  }
  else if (argc > 1 && strcmp(argv[1], "-opennt") == 0) {
    GetMounts(0);
  }
  else {
    fprintf(stderr,
	    "%s: must specify either -gnuwin32 or -opennt as first argument",
	    argv[0]);
    return 1;
  }

  argv++;
  argc--;

  argv[0] = "lib";

  printf("lib");

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {

      default:
	break;

      }

    } else {

      /* just a file argument */

      if (i == 1) {

	/* first arg is library name */

	int len = strlen(argv[i]);

	char *tmp = malloc(len + 6);
	sprintf(tmp, "/OUT:%s", argv[i]);
	argv[i] = TranslateFileName(tmp,5);

      } else {

	argv[i] = TranslateFileName(argv[i],0);

      }
    }

    if (strchr(argv[i], '"')) {
      argv[i] = EscapeDoubleQuotes(argv[i]);
    }

    printf(" %s", argv[i]);
  }

  printf("\n");

  rc = spawnvp(_P_WAIT,argv[0],argv);
  if (rc == -1) {
    fprintf(stderr,"%s: ",argv[0]);
    perror("");
    return 1;
  }
  return rc;
}
