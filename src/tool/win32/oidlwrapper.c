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
#include <stdio.h>
#include <process.h>
#include <string.h>
#include <assert.h>

#define WRAPPEREXE "oidlwrapper.exe"
#define UNWRAPPED  "omniidl"
#define MAX_MOUNTS 256

int  GetCygwinMounts(void);
void GetGnuwin32Mounts(void);
void SortMounts();
void GetOpenNTMounts(void);
void TranslatePathVar(char *var);
char *TranslateFileName(char *in, int offset);
char *EscapeDoubleQuotes(char *in);
int gnuwin32 = 0;
int opennt = 0;

int main(int argc, char **argv)
{
  int rc;
  int i;
  char *newprog;
  char *tprog;

  if (argc > 1 && strcmp(argv[1], "-gnuwin32") == 0) {
    if (!GetCygwinMounts())
      GetGnuwin32Mounts();
    SortMounts();
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

char *dos[MAX_MOUNTS];
char *unix[MAX_MOUNTS];
int index[MAX_MOUNTS];
int nmounts;

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
