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

#define MAX_MOUNTS 256

void GetGnuwin32Mounts(void);
void GetOpenNTMounts(void);
char *TranslateFileName(char *in, int offset);
char *EscapeDoubleQuotes(char *in);
int gnuwin32 = 0;
int opennt = 0;

int main(int argc, char **argv)
{
  int rc;
  int i;

  if (argc > 1 && strcmp(argv[1], "-gnuwin32") == 0) {
    GetGnuwin32Mounts();
    gnuwin32 = 1;
  } else if (argc > 1 && strcmp(argv[1], "-opennt") == 0) {
    GetOpenNTMounts();
    opennt = 1;
  } else {
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


char *EscapeDoubleQuotes(char *in)
{
  int i, j;
  char *out = malloc(strlen(in) * 2 + 1);

  j = 0;
  for (i = 0; i < strlen(in); i++) {
    if (in[i] == '"') {
      out[j++] = '\\';
    }
    out[j++] = in[i];
  }

  out[j] = '\0';

  return out;
}


char *dos[MAX_MOUNTS];
char *unix[MAX_MOUNTS];
int nmounts;

char *TranslateFileName(char *in, int offset)
{
  int i;
  char *out = NULL;

  for (i = 0; i < nmounts; i++) {
    if (strncmp(unix[i], &in[offset], strlen(unix[i])) == 0) {
      out = malloc(strlen(in) - strlen(unix[i]) + strlen(dos[i]) + 1);
      strncpy(out, in, offset);
      strcpy(out + offset, dos[i]);
      strcat(out, &in[offset + strlen(unix[i])]);
      break;
    }
  }

  if (!out) {
    out = malloc(strlen(in) + 1);
    strcpy(out, in);
  }

  for (i = offset; i < strlen(out); i++) {
    if (out[i] == '/') {
      out[i] = '\\';
    }
  }

  return out;
}

void GetOpenNTMounts(void)
{
  char c;

  nmounts = 0;

  for (c = 'A'; c <= 'Z'; c++) {
    unix[nmounts] = (char *)malloc(5);
    sprintf(unix[nmounts], "//%c/", c);
    dos[nmounts] = (char *)malloc(4);
    sprintf(dos[nmounts], "%c:/", c);
    nmounts++;
  }

  for (c = 'a'; c <= 'z'; c++) {
    unix[nmounts] = (char *)malloc(5);
    sprintf(unix[nmounts], "//%c/", c);
    dos[nmounts] = (char *)malloc(4);
    sprintf(dos[nmounts], "%c:/", c);
    nmounts++;
  }
}

void GetGnuwin32Mounts(void)
{
  HKEY hkey;
  LONG rc;
  int len;
  char key[256];

  for (nmounts = 0; nmounts < MAX_MOUNTS; nmounts++) {
    sprintf(key,
	    "Software\\Cygnus Solutions\\CYGWIN.DLL setup\\b15.0\\mounts\\%02X"
	    ,nmounts);

    if (RegOpenKeyEx(HKEY_CURRENT_USER,
		     key, 0, KEY_READ, &hkey) != ERROR_SUCCESS) {
      if (nmounts == 0) {
	printf("Couldn't find registry key '%s' - RegOpenKey error %d\n",
	       key, GetLastError());
	exit(1);
      }
      break;
    }

    if (RegQueryValueEx(hkey,"unix",NULL,NULL,NULL,&len) != ERROR_SUCCESS) {
      printf("RegQueryValueEx failed - error %d\n",GetLastError());
      exit(1);
    }
    unix[nmounts] = (char *)malloc(len+1);
    RegQueryValueEx(hkey, "unix", NULL, NULL, unix[nmounts], &len);

    if (RegQueryValueEx(hkey,"native",NULL,NULL,NULL,&len) != ERROR_SUCCESS) {
      printf("RegQueryValueEx failed - error %d\n",GetLastError());
      exit(1);
    }
    if (strcmp(unix[nmounts], "/") == 0) {
      dos[nmounts] = (char *)malloc(len+2);
      RegQueryValueEx(hkey, "native", NULL, NULL, dos[nmounts], &len);
      dos[nmounts][len-1] = '\\';
      dos[nmounts][len] = 0;
    } else {
      dos[nmounts] = (char *)malloc(len+1);
      RegQueryValueEx(hkey, "native", NULL, NULL, dos[nmounts], &len);
    }
  }
}
