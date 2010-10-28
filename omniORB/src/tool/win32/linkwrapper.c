/*
 * linkwrapper.c - wrapper around Visual C++'s "LINK" program to perform
 * file name translation (from unix-like to dos-like names).
 *
 * Both GNUWIN32 and OpenNT file names are supported - the first argument
 * to this program specifies which.
 *
 * Compile this program with "cl linkwrapper.c advapi32.lib".
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

  argv[0] = "link";

  printf("link");

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {

      if (strncmp(argv[i], "-libpath:", 9) == 0) {

	argv[i] = TranslateFileName(argv[i],9);

      } else if (strncmp(argv[i], "-out:", 5) == 0) {

	argv[i] = TranslateFileName(argv[i],5);

      } else if (strncmp(argv[i], "-def:", 5) == 0) {

	argv[i] = TranslateFileName(argv[i],5);

      }

    } else {

      /* just a file argument */

      argv[i] = TranslateFileName(argv[i],0);

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
