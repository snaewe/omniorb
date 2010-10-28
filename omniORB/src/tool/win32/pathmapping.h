/*
 * Common path mapping functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <assert.h>

//#define DEBUG_MOUNTS

#define MAX_MOUNTS 256

#ifdef __MINGW32__
void GetMinGW32Mounts(void);
#else
int  GetCygwinMounts(void);
void GetGnuwin32Mounts(void);
#endif
void GetOpenNTMounts(void);
void SortMounts();

char *dos[MAX_MOUNTS];
char *unix[MAX_MOUNTS];
int index[MAX_MOUNTS];
int nmounts;
char* drvprefix;


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


char *TranslateFileName(char *in, int offset)
{
  int i;
  char *out = NULL;

  for (i = 0; i < nmounts; i++) {
    char *upath = unix[index[i]];
    char *dpath = dos[index[i]];
    if (strncmp(upath, &in[offset], strlen(upath)) == 0) {
      out = malloc(strlen(in) - strlen(upath) + strlen(dpath) + 1);
      strncpy(out, in, offset);
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


/*  
 *  Convert the mixed UNIX/DOS path in environment variable "var" to
 *  all-DOS conventions.
 *
 *  For example, under OpenNT, if PYTHONPATH is
 *
 *      //c/foo/bar:c:\some\where;//h/bar/baz:c:\another\dir
 *
 *  then TranslatePathVar("PYTHONPATH") updates PYTHONPATH to
 *
 *      c:\foo\bar;c:\some\where;h:\bar\baz;c:\another\dir;
 *
 *  This allows makefiles to add components to paths unix-wise
 *  regardless of the conventions used by the previous value.
 */
typedef struct pathcomp {
    struct pathcomp	*next;
    char		*comp;
} PathComp;

#define NEW(TY)		((TY *) malloc (sizeof(TY)))

void TranslatePathVar (char *var)
{
    char	*oldpath = getenv (var);
    char 	*l, *r, *rhs;
    PathComp	*newpath = 0;
    PathComp	**npp = &newpath;
    PathComp	*c;
    int		pathlen = strlen (var) + 1; /* "var=" */
    char	*newvar;
    char	*nvp;
    char	tmp;

    if (!oldpath || !strlen (oldpath))
	return;

    /* Parse the "oldpath" into its components, translating to DOS. */
    rhs = oldpath + strlen(oldpath);
    for (l = oldpath; l < rhs; l = r + 1) {
	/* Skip "r" past single drive letters. */
	char *sep = l;
	while ((r = strpbrk (sep, ":;")) && *r == ':' && r == l + 1)
	    sep = r + 1;
	if (!r)
	    r = l + strlen (l);

	assert (oldpath <= l && l < rhs);
	assert (oldpath <= r && r <= rhs);

	/* "l" and "r" now point to the start and end of a path component.
	   Append its translation to the "newpath" list. */
	c = NEW (PathComp);
	c->next = 0;
	tmp = *r; *r = 0;
	c->comp = TranslateFileName (l, 0);
	*r = tmp;
	pathlen += strlen (c->comp) + 1; /* "comp;" */
	*npp = c;
	npp = &c->next;
    }

    /* Build the new environment variable "var=comp1;comp2;..;compn;". */
    newvar = malloc (pathlen + 1); /* allow for NUL */
    strcpy (newvar, var);
    strcat (newvar, "=");
    nvp = newvar;
    while (newpath) {
	nvp += strlen (nvp);
	strcpy (nvp, newpath->comp);
	strcat (nvp, ";");
	newpath = newpath->next;
    }
    _putenv (newvar);
}


void GetMounts(int gnuwin)
{
  drvprefix = "";
  if (gnuwin) {
#ifdef __MINGW32__
    GetMinGW32Mounts();
#else
    if (!GetCygwinMounts())
      GetGnuwin32Mounts();
#endif
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

#ifdef __MINGW32__
void GetMinGW32Mounts(void)
{
  HKEY hkey;
  char *msys_path,*fstab_name;
  int len,c;
  FILE *fstab;
  const char *key = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\MSYS-1.0_is1";

  nmounts = 0;

  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		   key, 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
    if (RegQueryValueEx(hkey,"Inno Setup: App Path",NULL,NULL,NULL,&len) != ERROR_SUCCESS) {
      printf("RegQueryValueEx failed - error %d\n",GetLastError());
      exit(1);
    }
  
    msys_path = (char *)malloc(len+2);

    if (RegQueryValueEx(hkey,"Inno Setup: App Path",NULL,NULL,msys_path,&len) != ERROR_SUCCESS) {
      printf("RegQueryValueEx failed - error %d\n",GetLastError());
      exit(1);
    }

    len = strlen(msys_path);
    if (len && msys_path[len-1] != '\\')
      {
	msys_path[len] = '\\';
	++len;
	msys_path[len] = '\0';
      }

    // add default mounts.
    unix[nmounts] = strdup("/");
    dos[nmounts] = msys_path;
    ++nmounts;

    unix[nmounts] = strdup("/usr/");
    dos[nmounts] = msys_path;
    ++nmounts;

    fstab_name = (char *)malloc(len+10);
    strcpy(fstab_name,msys_path);
    strcat(fstab_name,"etc\\fstab");

    // now scan /etc/fstab
    fstab = fopen(fstab_name,"rb");

    free(fstab_name);

    if (fstab) {
      char u[1024],d[1024];

      while (nmounts < MAX_MOUNTS) {

	c = fgetc(fstab);

	if (c != '#') {
	  while (c==' ' || c == '\t')
	    c = fgetc(fstab);

	  if (c != '\n') {
	    ungetc(c,fstab);
	    
	    if (fscanf (fstab,"%1024s %1024s",d,u) == 2) {
	      unix[nmounts] = strdup(u);
	      dos[nmounts] = strdup(d);
	      ++nmounts;
	    }
	    else
	      break;
	    while (c==' ' || c == '\t')
	      c = fgetc(fstab);
	    
	  }
	}
	else {
	  while (c != EOF && c != '\n')
	    c = fgetc(fstab);
	}
      
	if (c!= '\n') break;
      }

      fclose(fstab);
    }
  }

  // drive mounts.
  for (c = 'A'; c <= 'Z'; c++) {
    unix[nmounts] = (char *)malloc(4);
    sprintf(unix[nmounts], "/%c/", c);
    dos[nmounts] = (char *)malloc(4);
    sprintf(dos[nmounts], "%c:/", c);
    index[nmounts] = nmounts;
    nmounts++;
  }

  for (c = 'a'; c <= 'z'; c++) {
    unix[nmounts] = (char *)malloc(4);
    sprintf(unix[nmounts], "/%c/", c);
    dos[nmounts] = (char *)malloc(4);
    sprintf(dos[nmounts], "%c:/", c);
    index[nmounts] = nmounts;
    nmounts++;
  }

}

#else
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
    char        c;
    char*       drvprefix;

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &hkey)
	== ERROR_SUCCESS) {
      nmounts = 0;
  
      if (RegQueryValueEx (hkey, "cygdrive prefix", NULL, NULL, NULL, &len)
  	    == ERROR_SUCCESS) {
  
        drvprefix = (char *) malloc (len + 2);
        RegQueryValueEx (hkey, "cygdrive prefix", NULL, NULL, drvprefix, &len);
        if (drvprefix[strlen(drvprefix)-1] == '/')
	  drvprefix[strlen(drvprefix)-1] = '\0';
      
        for (c = 'A'; c <= 'Z'; c++) {
	  unix[nmounts] = (char *)malloc(strlen(drvprefix) + 4);
	  sprintf(unix[nmounts], "%s/%c/", drvprefix, c);
	  dos[nmounts] = (char *)malloc(4);
	  sprintf(dos[nmounts], "%c:/", c);
	  index[nmounts] = nmounts;
	  nmounts++;
        }
  
        for (c = 'a'; c <= 'z'; c++) {
	  unix[nmounts] = (char *)malloc(strlen(drvprefix) + 4);
	  sprintf(unix[nmounts], "%s/%c/", drvprefix, c);
	  dos[nmounts] = (char *)malloc(4);
	  sprintf(dos[nmounts], "%c:/", c);
	  index[nmounts] = nmounts;
	  nmounts++;
        }
  
      }
  
      for (i = 0; nmounts < MAX_MOUNTS; i++) {
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
  	unix[nmounts] = (char *) malloc (upathlen + 1);
  	strcpy (unix[nmounts], upath);
  	if (RegQueryValueEx (hkey1, "native", NULL, NULL, NULL, &len)
  	    != ERROR_SUCCESS) {
	  printf("RegQueryValueEx failed - error %d\n",GetLastError());
	  exit(1);
  	}
  	if (strcmp (unix[nmounts], "/") == 0) {
	  dos[nmounts] = (char *) malloc (len + 2);
	  RegQueryValueEx (hkey1, "native", NULL, NULL, dos[nmounts], &len);
	  dos[nmounts][len-1] = '\\';
	  dos[nmounts][len] = 0;
  	}
	else {
	  dos[nmounts] = (char *) malloc (len + 1);
	  RegQueryValueEx (hkey1, "native", NULL, NULL, dos[nmounts], &len);
  	}
  	nmounts++;
      }
      return 1;
    }
    else {
      /* Newer cygwin does not use the registry to store mounts. Run
	 the mount command instead. */
      FILE* mounts;
      nmounts = 0;
      mounts = _popen("mount", "r");

      if (mounts) {
        char dpath[256];
        char upath[256];
        while (nmounts < MAX_MOUNTS) {
          int idx;
	  int ret = fscanf(mounts, "%256s on %256s", dpath, upath);
	  if (ret == EOF)
	    break;
	  else if (ret < 2)
	    continue;

  	  unix[nmounts] = (char *) malloc (strlen(upath) + 1);
  	  strcpy (unix[nmounts], upath);

          if (strcmp(unix[nmounts], "/") == 0) {
  	    dos[nmounts] = (char *) malloc (strlen(dpath) + 2);
  	    strcpy (dos[nmounts], dpath);
            dos[nmounts][strlen(dpath)] = '/';
            dos[nmounts][strlen(dpath) + 1] = '\0';
          }
	  else {
  	    dos[nmounts] = (char *) malloc (strlen(dpath) + 1);
  	    strcpy (dos[nmounts], dpath);
          }

          for (idx = 0; dos[nmounts][idx]; ++idx) {
            if (dos[nmounts][idx] == '/') {
              dos[nmounts][idx] = '\\';
            }
          }
          ++nmounts;
        }
        fclose(mounts);
      }

      if (0 != nmounts) {
        return 1;
      }
    }

    return 0;
}
#endif

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

#ifdef DEBUG_MOUNTS
    for (i = 0; i < nmounts; i++)
      printf("SortMounts: i,unix,dos = %d,%s,%s.\n",
	      i,unix[index[i]],dos[index[i]]);
#endif
}
