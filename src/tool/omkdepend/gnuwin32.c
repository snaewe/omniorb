
#include <windows.h>
#include "gnuwin32.h"


#define MAX_MOUNTS 256

int  GetCygwinMounts(void);
void GetGnuwin32Mounts(void);
void GetOpenNTMounts(void);
void SortMounts();

char *dos[MAX_MOUNTS];
char *unix[MAX_MOUNTS];
int index[MAX_MOUNTS];
int nmounts;

char *TranslateFileNameU2D(char *in, int offset)
{
  int i;
  char *out = NULL;

  for (i = 0; i < nmounts; i++) {
    char *upath = unix[index[i]];
    char *dpath = dos[index[i]];
    if (strncmp(upath, &in[offset], strlen(upath)) == 0) {
      out = malloc(strlen(in) - strlen(upath) + strlen(dpath) + 1);
      strncpy(out, in, offset); /* doesn't NUL-terminate! */
      strcpy(out + offset, dpath);
      strcat(out, &in[offset + strlen(upath)]);
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

char *TranslateFileNameD2U(char *in, int offset)
{
  int i;
  char *out = NULL;

  for (i = 0; i < nmounts; i++) {
    char *upath = unix[index[i]];
    char *dpath = dos[index[i]];
    if (strncmp(dpath, &in[offset], strlen(dpath)) == 0) {
      out = malloc(strlen(in) - strlen(dpath) + strlen(upath) + 1);
      strncpy(out, in, offset); /* doesn't NUL-terminate! */
      strcpy(out + offset, upath);
      strcat(out, &in[offset + strlen(dpath)]);
      break;
    }
  }

  if (!out) {
    out = malloc(strlen(in) + 1);
    strcpy(out, in);
  }

  for (i = offset; i < strlen(out); i++) {
    if (out[i] == '\\') {
      out[i] = '/';
    }
  }

  return out;
}

void GetMounts(int gnuwin)
{
  if (gnuwin) {
    if (!GetCygwinMounts())
      GetGnuwin32Mounts();
  }
  else {
    GetOpenNTMounts();
  }
  SortMounts();
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
    index[nmounts] = nmounts;
    nmounts++;
  }

  for (c = 'a'; c <= 'z'; c++) {
    unix[nmounts] = (char *)malloc(5);
    sprintf(unix[nmounts], "//%c/", c);
    dos[nmounts] = (char *)malloc(4);
    sprintf(dos[nmounts], "%c:/", c);
    index[nmounts] = nmounts;
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

int
GetCygwinMounts()
{
    static char	key[] = "Software\\Cygnus Solutions\\Cygwin\\mounts v2";
    HKEY	hkey;
    HKEY	hkey1;
    char	upath[1024];
    DWORD	upathlen;
    DWORD	len;
    DWORD	i;
    LONG	rc;

    if (RegOpenKeyEx (HKEY_CURRENT_USER, key, 0, KEY_READ, &hkey)
	!= ERROR_SUCCESS)
	return 0;

    for (i = 0; i < MAX_MOUNTS; i++) {
	upathlen = sizeof(upath);
	rc = RegEnumKeyEx (hkey, i, upath, &upathlen, NULL, NULL, NULL, NULL);
	if (rc == ERROR_NO_MORE_ITEMS)
	    break;
	if (rc != ERROR_SUCCESS) {
	    printf ("RegEnumKeyEx(%d) failed - error %d\n", i, GetLastError());
	    exit(1);
	}
	if (RegOpenKeyEx (hkey, upath, 0, KEY_READ, &hkey1)
	    != ERROR_SUCCESS) {
	    printf ("RegOpenKeyEx() failed - error %d\n", GetLastError());
	    exit(1);
	}
	unix[i] = (char *) malloc (upathlen + 1);
	strcpy (unix[i], upath);
	if (RegQueryValueEx (hkey1, "native", NULL, NULL, NULL, &len)
	    != ERROR_SUCCESS) {
	    printf("RegQueryValueEx failed - error %d\n",GetLastError());
	    exit(1);
	}
	if (strcmp (unix[i], "/") == 0) {
	    dos[i] = (char *) malloc (len + 2);
	    RegQueryValueEx (hkey1, "native", NULL, NULL, dos[i], &len);
	    dos[i][len-1] = '\\';
	    dos[i][len] = 0;
	} else {
	    dos[i] = (char *) malloc (len + 1);
	    RegQueryValueEx (hkey1, "native", NULL, NULL, dos[i], &len);
	}
    }
    nmounts = i;
    return 1;
}

int
longest_first (const void *pi, const void *pj)
{
    int i = *(int *) pi;
    int j = *(int *) pj;
    return strlen (unix[j]) - strlen (unix[i]);
}

void
SortMounts()
{
    int i;

    for (i = 0; i < nmounts; i++)
	index[i] = i;
    qsort (index, nmounts, sizeof(int), longest_first);
}
