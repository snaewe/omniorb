
#include <windows.h>
#include "gnuwin32.h"


#define MAX_MOUNTS 256

char *dos[MAX_MOUNTS];
char *unix[MAX_MOUNTS];
int nmounts;

char *TranslateFileNameU2D(char *in, int offset)
{
  int i;
  char *out = NULL;

  for (i = 0; i < nmounts; i++) {
    if (strncmp(unix[i], &in[offset], strlen(unix[i])) == 0) {
      out = malloc(strlen(in) - strlen(unix[i]) + strlen(dos[i]) + 1);
      out[0] = '\0';
      strncpy(out, in, offset);
      strcat(out, dos[i]);
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

char *TranslateFileNameD2U(char *in, int offset)
{
  int i;
  char *out = NULL;

  for (i = 0; i < nmounts; i++) {
    if (strncmp(dos[i], &in[offset], strlen(dos[i])) == 0) {
      out = malloc(strlen(in) - strlen(dos[i]) + strlen(unix[i]) + 1);
      out[0] = '\0';
      strncpy(out, in, offset);
      strcat(out, unix[i]);
      strcat(out, &in[offset + strlen(dos[i])]);
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

void GetMounts(void)
{
  HKEY hkey;
  LONG rc;
  int len;
  char key[256];

  for (nmounts = 0; nmounts < MAX_MOUNTS; nmounts++) {
    sprintf(key,
	    "Software\\Cygnus Support\\CYGWIN.DLL setup\\b14.0\\mounts\\%02d",
	    nmounts);

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
    unix[nmounts] = (char *)malloc(len);
    RegQueryValueEx(hkey, "unix", NULL, NULL, unix[nmounts], &len);

    if (RegQueryValueEx(hkey,"dos",NULL,NULL,NULL,&len) != ERROR_SUCCESS) {
      printf("RegQueryValueEx failed - error %d\n",GetLastError());
      exit(1);
    }
    if (strcmp(unix[nmounts], "/") == 0) {
      dos[nmounts] = (char *)malloc(len + 1);
      RegQueryValueEx(hkey, "dos", NULL, NULL, dos[nmounts], &len);
      dos[nmounts][len-1] = '\\';
      dos[nmounts][len] = 0;
    } else {
      dos[nmounts] = (char *)malloc(len);
      RegQueryValueEx(hkey, "dos", NULL, NULL, dos[nmounts], &len);
    }
  }
}
