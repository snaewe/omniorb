/*
 * IMPORTANT - compile this with gnu-win32's gcc, not MS's cl.
 * gcc omake.c -o omake.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define MAX_ARGS 256

void AddToString(char **strp, char *new);


int main(int argc, char **argv)
{
  char top[FILENAME_MAX];
  char current[FILENAME_MAX];
  char currentAbsolute[FILENAME_MAX];
  char tmp[FILENAME_MAX];
  char source[FILENAME_MAX];
  int firstSource = 1;
  char *vpath = NULL;
  char *lastSlash;
  char *makeCmd[MAX_ARGS];
  int nArgs = 0;
  FILE *f;
  int i;
  int quiet = 0;

  if ((argc >= 2) && (strcmp(argv[1],"-quiet") == 0)) {
    quiet = 1;
    argv++;
  }

  getcwd(currentAbsolute,FILENAME_MAX);

  if (strcmp(currentAbsolute,"/") == 0) {
    fprintf(stderr,"ERROR: NOT IN AN OMNI TREE\n");
    exit(1);
  }

  strcpy(top,".");
  strcpy(current,".");

  while (1) {

    if (strcmp(currentAbsolute,"") == 0) {
      fprintf(stderr,"ERROR: NOT IN AN OMNI TREE\n");
      exit(1);
    }

    sprintf(tmp,"%s/config/config.mk",top);

    if (access(tmp, R_OK) == 0) {
      sprintf(tmp,"%s/config/sources",top);

      if (access(tmp, R_OK) == 0) {
	break;
      }
    }

    if (strcmp(top,".") == 0) {

      strcpy(top,"..");

      lastSlash = strrchr(currentAbsolute,'/');
      strcpy(current,lastSlash+1);

      *lastSlash = '\0';	/* strip end off currentAbsolute */

    } else {

      strcpy(tmp,top);
      sprintf(top,"../%s",tmp);

      strcpy(tmp,current);
      lastSlash = strrchr(currentAbsolute,'/');
      sprintf(current,"%s/%s",lastSlash+1,tmp);

      *lastSlash = '\0';	/* strip end off currentAbsolute */
    }
  }

  makeCmd[nArgs++] = "make";
  makeCmd[nArgs++] = "-r";
  makeCmd[nArgs++] = "-f";
  sprintf(tmp,"%s/config/config.mk",top);
  makeCmd[nArgs++] = strdup(tmp);

  sprintf(tmp,"%s/config/sources",top);
  f = fopen(tmp,"r");

  while (fgets(source, FILENAME_MAX, f)) {
    if (source[strlen(source)-1] == '\n') {
      source[strlen(source)-1] = '\0';
    }

    if (source[0] != '/') {	/* source has relative path */
      strcpy(tmp,source);
      sprintf(source,"%s/%s",top,tmp);
    }

    strcpy(tmp,source);
    sprintf(source,"%s/%s",tmp,current);

    if (firstSource) {
      AddToString(&vpath, "VPATH=");
      firstSource = 0;
    } else {
      AddToString(&vpath, ":");
    }

    AddToString(&vpath, source);

    makeCmd[nArgs++] = "-I";
    makeCmd[nArgs++] = strdup(source);
  }

  fclose(f);

  makeCmd[nArgs++] = vpath;

  sprintf(tmp,"TOP=%s",top);
  makeCmd[nArgs++] = strdup(tmp);

  sprintf(tmp,"CURRENT=%s",current);
  makeCmd[nArgs++] = strdup(tmp);

  for (i = 1; i < argc; i++) {
    makeCmd[nArgs++] = argv[i];
  }

  if (!quiet) {
    for (i = 0; i < nArgs; i++) {
      printf("%s ",makeCmd[i]);
    }

    printf("\n");
  }

  if (execvp(makeCmd[0],makeCmd) < 0) {
    perror("execvp");
    return 1;
  }

  return 0;
}


void AddToString(char **strp, char *new)
{
  if (*strp == NULL) {
    *strp = malloc(strlen(new) + 1);
    *strp[0] = '\0';
  } else {
    *strp = realloc(*strp, strlen(*strp) + strlen(new) + 1);
  }

  strcat(*strp,new);
}
