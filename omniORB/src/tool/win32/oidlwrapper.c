/*
 * oidlwrapper.c - wrapper round omniidl to perform
 * file name translation (from unix-like to dos-like names).
 *
 * Both GNUWIN32 and OpenNT file names are supported - the first argument
 * to this program specifies which.
 *
 * Compile this program with "cl oidlwrapper.c advapi32.lib".
 */

#include <windows.h>
#include <process.h>
#include <string.h>

#include "pathmapping.h"

#define WRAPPEREXE "oidlwrapper.exe"
#define UNWRAPPED  "omniidl"

int main(int argc, char **argv)
{
  int rc;
  int i;
  char *newprog;
  char *tprog;

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

  TranslatePathVar("PYTHONPATH");
  printf ("PYTHONPATH=%s\n", getenv("PYTHONPATH"));

  /* Make sure we use the omniidl next to this wrapper rather than
     a random one on PATH */
  tprog = TranslateFileName(argv[0], 0);
  newprog = UNWRAPPED; /* lookup on PATH by default */

  assert (sizeof(WRAPPEREXE) > sizeof(UNWRAPPED));

  if (strlen(tprog) > strlen(WRAPPEREXE)) {
      char *tail = tprog + strlen(tprog) - strlen(WRAPPEREXE);
      if (stricmp (tail, WRAPPEREXE) == 0) {
	  strcpy (tail, UNWRAPPED);
	  newprog = tprog;
      }
  }

  argv++;
  argc--;

  argv[0] = newprog;

  printf(newprog);

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {

      case 'I': case 'C': case 'p':
	argv[i] = TranslateFileName(argv[i],2);
	break;

      default:
	break;

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
