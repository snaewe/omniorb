#include <stdio.h>
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX BUFSIZ
#ifndef __cplusplus
# define output(c) (void)putc(c,yyout)
#else
# define lex_output(c) (void)putc(c,yyout)
#endif

#if defined(__cplusplus)
#ifndef __EXTERN_C__
#define __EXTERN_C__
#endif
#endif

#if defined(__cplusplus) || defined(__STDC__)

#if defined(__cplusplus) && defined(__EXTERN_C__)
extern "C" {
#endif
	int yyback(int *, int);
	int yyinput(void);
	int yylook(void);
	void yyoutput(int);
	int yyracc(int);
	int yyreject(void);
	void yyunput(int);
	int yylex(void);
#ifdef YYLEX_E
	void yywoutput(wchar_t);
	wchar_t yywinput(void);
#endif
#ifndef yyless
	int yyless(int);
#endif
#ifndef yywrap
	int yywrap(void);
#endif
#ifdef LEXDEBUG
	void allprint(char);
	void sprint(char *);
#endif
#if defined(__cplusplus) && defined(__EXTERN_C__)
}
#endif

#if !defined(__WIN32__)

#ifdef __cplusplus
extern "C" {
#endif
	void exit(int);
#ifdef __cplusplus
}
#endif

#endif // __WIN32__

#endif
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
#ifndef __cplusplus
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#else
# define lex_input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#endif
#define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng;
char yytext[YYLMAX];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
#ifndef __DECCXX
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
#endif

# line 4 "idl.ll"
/*

COPYRIGHT

Copyright 1992, 1993, 1994 Sun Microsystems, Inc.  Printed in the United
States of America.  All Rights Reserved.

This product is protected by copyright and distributed under the following
license restricting its use.

The Interface Definition Language Compiler Front End (CFE) is made
available for your use provided that you include this license and copyright
notice on all media and documentation and the software program in which
this product is incorporated in whole or part. You may copy and extend
functionality (but may not remove functionality) of the Interface
Definition Language CFE without charge, but you are not authorized to
license or distribute it to anyone else except as part of a product or
program developed by you or with the express written consent of Sun
Microsystems, Inc. ("Sun").

The names of Sun Microsystems, Inc. and any of its subsidiaries or
affiliates may not be used in advertising or publicity pertaining to
distribution of Interface Definition Language CFE as permitted herein.

This license is effective until terminated by Sun for failure to comply
with this license.  Upon termination, you shall destroy or return all code
and documentation for the Interface Definition Language CFE.

INTERFACE DEFINITION LANGUAGE CFE IS PROVIDED AS IS WITH NO WARRANTIES OF
ANY KIND INCLUDING THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS
FOR A PARTICULAR PURPOSE, NONINFRINGEMENT, OR ARISING FROM A COURSE OF
DEALING, USAGE OR TRADE PRACTICE.

INTERFACE DEFINITION LANGUAGE CFE IS PROVIDED WITH NO SUPPORT AND WITHOUT
ANY OBLIGATION ON THE PART OF Sun OR ANY OF ITS SUBSIDIARIES OR AFFILIATES
TO ASSIST IN ITS USE, CORRECTION, MODIFICATION OR ENHANCEMENT.

SUN OR ANY OF ITS SUBSIDIARIES OR AFFILIATES SHALL HAVE NO LIABILITY WITH
RESPECT TO THE INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY
INTERFACE DEFINITION LANGUAGE CFE OR ANY PART THEREOF.

IN NO EVENT WILL SUN OR ANY OF ITS SUBSIDIARIES OR AFFILIATES BE LIABLE FOR
ANY LOST REVENUE OR PROFITS OR OTHER SPECIAL, INDIRECT AND CONSEQUENTIAL
DAMAGES, EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

Use, duplication, or disclosure by the government is subject to
restrictions as set forth in subparagraph (c)(1)(ii) of the Rights in
Technical Data and Computer Software clause at DFARS 252.227-7013 and FAR
52.227-19.

Sun, Sun Microsystems and the Sun logo are trademarks or registered
trademarks of Sun Microsystems, Inc.

SunSoft, Inc.  
2550 Garcia Avenue 
Mountain View, California  94043

NOTE:

SunOS, SunSoft, Sun, Solaris, Sun Microsystems or the Sun logo are
trademarks or registered trademarks of Sun Microsystems, Inc.

 */


# line 68 "idl.ll"
/*
 * idl.ll - Lexical scanner for IDL 1.1
 */

#include <idl.hh>
#include <idl_extern.hh>

#include <fe_private.hh>

#if defined(__WIN32__)
#include "y.tab.hh"
#elif defined(__VMS)
#include "y_tab.hh"
#else
#include <y.tab.hh>
#endif

#include <string.h>

static char	idl_escape_reader(char *);
static double	idl_atof(char *);
static long	idl_atoi(char *, long);
static void	idl_parse_line_and_file(char *);
static void	idl_parse_line_and_file_NT(char *);
static void	idl_store_pragma(char *);

// HPUX has yytext typed to unsigned char *. We make sure here that
// we'll always use char *
static char	*__yytext = (char *) yytext;

# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
#ifdef __cplusplus
/* to avoid CC and lint complaining yyfussy not being used ...*/
static int __lex_hack = 0;
if (__lex_hack) goto yyfussy;
#endif
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:

# line 102 "idl.ll"
	return MODULE;
break;
case 2:

# line 103 "idl.ll"
	return RAISES;
break;
case 3:

# line 104 "idl.ll"
return READONLY;
break;
case 4:

# line 105 "idl.ll"
return ATTRIBUTE;
break;
case 5:

# line 106 "idl.ll"
return EXCEPTION;
break;
case 6:

# line 107 "idl.ll"
	return CONTEXT;
break;
case 7:

# line 108 "idl.ll"
return INTERFACE;
break;
case 8:

# line 109 "idl.ll"
	return CONST;
break;
case 9:

# line 110 "idl.ll"
	return TYPEDEF;
break;
case 10:

# line 111 "idl.ll"
	return STRUCT;
break;
case 11:

# line 112 "idl.ll"
	return ENUM;
break;
case 12:

# line 113 "idl.ll"
	return STRING;
break;
case 13:

# line 114 "idl.ll"
return WSTRING;
break;
case 14:

# line 115 "idl.ll"
return SEQUENCE;
break;
case 15:

# line 116 "idl.ll"
	return UNION;
break;
case 16:

# line 117 "idl.ll"
	return SWITCH;
break;
case 17:

# line 118 "idl.ll"
	return CASE;
break;
case 18:

# line 119 "idl.ll"
	return DEFAULT;
break;
case 19:

# line 120 "idl.ll"
	return FLOAT;
break;
case 20:

# line 121 "idl.ll"
	return DOUBLE;
break;
case 21:

# line 122 "idl.ll"
	return LONG;
break;
case 22:

# line 123 "idl.ll"
	return SHORT;
break;
case 23:

# line 124 "idl.ll"
return UNSIGNED;
break;
case 24:

# line 125 "idl.ll"
	return CHAR;
break;
case 25:

# line 126 "idl.ll"
	return WCHAR;
break;
case 26:

# line 127 "idl.ll"
	return BOOLEAN;
break;
case 27:

# line 128 "idl.ll"
	return OCTET;
break;
case 28:

# line 129 "idl.ll"
	return VOID;
break;
case 29:

# line 131 "idl.ll"
	return TRUETOK;
break;
case 30:

# line 132 "idl.ll"
	return FALSETOK;
break;
case 31:

# line 134 "idl.ll"
	return INOUT;
break;
case 32:

# line 135 "idl.ll"
	return IN;
break;
case 33:

# line 136 "idl.ll"
	return OUT;
break;
case 34:

# line 137 "idl.ll"
	return ONEWAY;
break;
case 35:

# line 139 "idl.ll"
	return LEFT_SHIFT;
break;
case 36:

# line 140 "idl.ll"
	return RIGHT_SHIFT;
break;
case 37:

# line 141 "idl.ll"
	{
		  yylval.strval = "::";    
		  return SCOPE_DELIMITOR;
		}
break;
case 38:

# line 146 "idl.ll"
{
    char *z = (char *) malloc(strlen(__yytext) + 1);
    strcpy(z, __yytext);
    yylval.strval = z;
    return IDENTIFIER;
}
break;
case 39:

# line 153 "idl.ll"
     {
                  yylval.dval = idl_atof(__yytext);
                  return FLOATING_PT_LITERAL;
                }
break;
case 40:

# line 157 "idl.ll"
 {
                  yylval.dval = idl_atof(__yytext);
                  return FLOATING_PT_LITERAL;
                }
break;
case 41:

# line 162 "idl.ll"
{
		  yylval.ival = idl_atoi(__yytext, 10);
		  return INTEGER_LITERAL;
	        }
break;
case 42:

# line 166 "idl.ll"
{
		  yylval.ival = idl_atoi(__yytext, 16);
		  return INTEGER_LITERAL;
	        }
break;
case 43:

# line 170 "idl.ll"
{
		  yylval.ival = idl_atoi(__yytext, 8);
		  return INTEGER_LITERAL;
	      	}
break;
case 44:

# line 175 "idl.ll"
{
		  __yytext[strlen(__yytext)-1] = '\0';
		  yylval.sval = new String(__yytext + 1);
		  return STRING_LITERAL;
	      	}
break;
case 45:

# line 180 "idl.ll"
	{
		  yylval.cval = __yytext[1];
		  return CHARACTER_LITERAL;
	      	}
break;
case 46:

# line 184 "idl.ll"
{
		  // octal character constant
		  yylval.cval = idl_escape_reader(__yytext + 1);
		  return CHARACTER_LITERAL;
		}
break;
case 47:

# line 189 "idl.ll"
{
		  yylval.cval = idl_escape_reader(__yytext + 1);
		  return CHARACTER_LITERAL;
		}
break;
case 48:

# line 194 "idl.ll"
{
                   return PRAGMA_VERSION;
                 }
break;
case 49:

# line 198 "idl.ll"
{
                    return PRAGMA_ID;
		 }
break;
case 50:

# line 202 "idl.ll"
{
                     return PRAGMA_PREFIX;
                 }
break;
case 51:

# line 206 "idl.ll"
{/* remember pragma */
		  idl_store_pragma(__yytext);
		}
break;
case 52:

# line 209 "idl.ll"
		{
		  idl_parse_line_and_file(__yytext);
		}
break;
case 53:

# line 212 "idl.ll"
{
		  idl_parse_line_and_file(__yytext);
	        }
break;
case 54:

# line 215 "idl.ll"
		{
		  idl_parse_line_and_file_NT(__yytext);
		}
break;
case 55:

# line 218 "idl.ll"
{
		  idl_parse_line_and_file_NT(__yytext);
	        }
break;
case 56:

# line 221 "idl.ll"
{
		  /* ignore cpp ident */
  		  idl_global->set_lineno(idl_global->lineno() + 1);
		}
break;
case 57:

# line 225 "idl.ll"
{
		  /* ignore comments */
  		  idl_global->set_lineno(idl_global->lineno() + 1);
		}
break;
case 58:

# line 229 "idl.ll"
	{
		  for(;;) {
		    char c = yyinput();
		    if (c == '*') {
		      char next = yyinput();
		      if (next == '/')
			break;
		      else
			/* yyunput(c); */
			unput(c);
	              if (c == '\n') 
		        idl_global->set_lineno(idl_global->lineno() + 1);
		    }
	          }
	        }
break;
case 59:

# line 244 "idl.ll"
	;
break;
case 60:

# line 245 "idl.ll"
	{
  		  idl_global->set_lineno(idl_global->lineno() + 1);
		}
break;
case 61:

# line 248 "idl.ll"
	return __yytext[0];
break;
case -1:
break;
default:
(void)fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
	/* subroutines */

/*
 * Strip down a name to the last component, i.e. everything after the last
 * '/' character
 */
static char *
stripped_name(UTL_String *fn)
{
    char	*n = fn->get_string();
    long	l;

    if (n == NULL)
	return NULL;
    l = strlen(n);
#if defined(__WIN32__)
    for (n += l; l > 0 && *n != 92; l--, n--);
    if (*n == 92) n++;
#elif defined(__VMS)

    for (n += l; l > 0 && *n != ';'; l--, n--);
    if (*n == ';') {
        static UTL_String temp;
        temp = UTL_String(fn);
        n = temp.get_string();
        n[l] = 0;
    }

    for (n += l; l > 0 && *n != ']' && *n != ':'; l--, n--);
    if (*n == ']' || *n==':') n++;

#else
    for (n += l; l > 0 && *n != '/'; l--, n--);
    if (*n == '/') n++;
#endif

    return n;
}

/*
 * Parse a #line statement generated by the C preprocessor
 */
static void
idl_parse_line_and_file(char *buf)
{
  char		*r = buf;
  char 		*h;
  UTL_String	*nm;

  /* Skip initial '#' */
  if (*r != '#') {
    return;
  }

  /* Find line number */
  for (r++; *r == ' ' || *r == '\t'; r++);
  h = r;
  for (; *r != '\0' && *r != ' ' && *r != '\t'; r++);
  *r++ = 0;
  idl_global->set_lineno(idl_atoi(h, 10));

  /* Find file name, if present */
  for (; *r != '"'; r++) {
    if (*r == '\n' || *r == '\0')
      return;
  }
  h = ++r;
  for (; *r != '"'; r++);
  *r = 0;
  if (*h == '\0')
    idl_global->set_filename(new String("standard input"));
  else {
#ifdef __VMS
    // kill version
    char* v(r);
    for(--v; v != h && *v != ';'; --v);
    if (*v==';') *v = 0;
    else v = r;
    // kill device/directory:
    for(r=v; r != h && *r != ']' && *r != ':'; --r) {
        if(isalpha(*r)) *r = tolower(*r);
    }
    if (*r==']' || *r==':') h = r+1;
#endif
    idl_global->set_filename(new String(h));
  }

  idl_global->set_in_main_file(
    (idl_global->filename()->compare(idl_global->real_filename())) ?
    I_TRUE :
    I_FALSE
  );
  /*
   * If it's an import file store the stripped name for the BE to use
   */
  if (!(idl_global->in_main_file()) && idl_global->import()) {
    nm = new UTL_String(stripped_name(idl_global->filename()));
    idl_global->store_include_file_name(nm);
  }
}


/*
 * Parse a #line statement generated by the C preprocessor
 * - NT version.
 */
static void
idl_parse_line_and_file_NT(char *buf)
{
  char		*r = buf;
  char 		*h;
  char 		*j;
  int count=0,jcount=0;
  UTL_String	*nm;

  /* Skip initial '#' */
  if (*r != '#') {
    return;
  }

  /* Find line number */

  /* Skip whitespace: */
  for (r++; *r == ' ' || *r == '\t'; r++);

  /* Skip "line": */
  r+=4;

  /* Skip whitespace: */
  for (r++; *r == ' ' || *r == '\t'; r++);

  h = r;
  for (; *r != '\0' && *r != ' ' && *r != '\t'; r++);
  *r++ = 0;
  idl_global->set_lineno(idl_atoi(h, 10));

  /* Find file name, if present */
  for (; *r != '"'; r++) {
    if (*r == '\n' || *r == '\0')
      return;
  }
  h = ++r;
  for (; *r != '"'; r++);
  *r = 0;

  if (*h == '\0')
    idl_global->set_filename(new String("standard input"));
  else
	{
	/* Get around the NT include problem: */

	  j = new char[strlen(h)+1];

  	  int nflag = 0;
	  for (count = 0; count < strlen(h); count++)
		{
		  if (h[count] == 92 && h[count+1] == 92 && nflag == 0)
			{
			nflag = 1;
			continue;
			}
		  nflag = 0;
		  j[jcount] = h[count];
		  jcount++;
		}
   	  j[jcount] = 0;
	  idl_global->set_filename(new String(j));
	  delete[] j;
	}


  idl_global->set_in_main_file(
    (idl_global->filename()->compare(idl_global->real_filename())) ?
    I_TRUE :
    I_FALSE
  );
  /*
   * If it's an import file store the stripped name for the BE to use
   */
  if (!(idl_global->in_main_file()) && idl_global->import()) {
    nm = new UTL_String(stripped_name(idl_global->filename()));
    idl_global->store_include_file_name(nm);
  }
}


/*
 * Store a #pragma line into the list of pragmas
 */
static void
idl_store_pragma(char *buf)
{
  char *cp = buf + 1;
  while(*cp != 'p')
    cp++;
  while(*cp != ' ' && *cp != '\t')
    cp++;
  while(*cp == ' ' || *cp == '\t')
    cp++;

  char *pp = cp;
  while (*pp != '\n') pp++;
  char* pragma = new char[pp-cp+1];
  pp = pragma;
  while(*cp != '\n') {
    *pp++ = *cp++;
  }
  *pp = 0;
  pp = strchr(pragma,' ');
  if (!pp) pp = strchr(pragma,'\t');
  if (strncmp(pragma,"version",7) == 0) {
    if (!pp || pp == pragma + 7) {
      idl_global->set_parse_state(IDL_GlobalData::PS_PragmaVersionSeen);
      idl_global->err()->syntax_error(idl_global->parse_state());
      idl_global->set_lineno(idl_global->lineno() + 1);
      return;
    }
  }
  else if (strncmp(pragma,"ID",2) == 0) {
    if (!pp || pp == pragma + 2) {
      idl_global->set_parse_state(IDL_GlobalData::PS_PragmaIDSeen);
      idl_global->err()->syntax_error(idl_global->parse_state());
      idl_global->set_lineno(idl_global->lineno() + 1);
      return;
    }
  }
  else if (strncmp(pragma,"prefix",6) == 0) {
    if (!pp || pp == pragma + 6) {
      idl_global->set_parse_state(IDL_GlobalData::PS_PragmaPrefixSeen);
      idl_global->err()->syntax_error(idl_global->parse_state());
      idl_global->set_lineno(idl_global->lineno() + 1);
      return;
    }
  }
  else if (strcmp(pragma, "import") == 0) {
    idl_global->set_import(I_TRUE);
    return;
  }
  else if (strcmp(pragma, "include") == 0) {
    idl_global->set_import(I_FALSE);
    return;
  }

  UTL_StrList *p = idl_global->pragmas();
  if (p == NULL)
    idl_global->set_pragmas(new UTL_StrList(new String(""),
	 	                            new UTL_StrList(new String(buf),
                                                           NULL
                                                           )
                                            )
                            );
  else {
    p->nconc(new UTL_StrList(new String(""),
                             new UTL_StrList(new String(buf),
                                             NULL
                                            )
	                    )
             );
    idl_global->set_pragmas(p);
  }
  idl_global->set_lineno(idl_global->lineno() + 1);
}

/*
 * idl_atoi - Convert a string of digits into an integer according to base b
 */
static long
idl_atoi(char *s, long b)
{
	long	r = 0;
	long	negative = 0;

	if (*s == '-') {
	  negative = 1;
	  s++;
	}
	if (b == 8 && *s == '0')
	  s++;
	else if (b == 16 && *s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X'))
	  s += 2;

	for (; *s; s++)
	  if (*s <= '9' && *s >= '0')
	    r = (r * b) + (*s - '0');
	  else if (b > 10 && *s <= 'f' && *s >= 'a')
	    r = (r * b) + (*s - 'a' + 10);
	  else if (b > 10 && *s <= 'F' && *s >= 'A')
	    r = (r * b) + (*s - 'A' + 10);
	  else
	    break;

	if (negative)
	  r *= -1;

	return r;
}

/*
 * Convert a string to a float; atof doesn't seem to work, always.
 */
static double
idl_atof(char *s)
{
	char    *h = s;
	double	d = 0.0;
	double	f = 0.0;
	double	e, k;
	long	neg = 0, negexp = 0;

	if (*s == '-') {
	  neg = 1;
	  s++;
	}
	while (*s >= '0' && *s <= '9') {
		d = (d * 10) + *s - '0';
		s++;
	}
	if (*s == '.') {
		s++;
		e = 10;
		while (*s >= '0' && *s <= '9') {
			d += (*s - '0') / (e * 1.0);
			e *= 10;
			s++;
		}
	}
	if (*s == 'e' || *s == 'E') {
		s++;
		if (*s == '-') {
			negexp = 1;
			s++;
		} else if (*s == '+')
			s++;
		e = 0;
		while (*s >= '0' && *s <= '9') {
			e = (e * 10) + *s - '0';
			s++;
		}
		if (e > 0) {
			for (k = 1; e > 0; k *= 10, e--);
			if (negexp)
				d /= k;
			else
				d *= k;
		}
	}

	if (neg) d *= -1.0;

	return d;
}

/*
 * Convert (some) escaped characters into their ascii values
 */
static char
idl_escape_reader(
    char *str
)
{
    if (str[0] != '\\') {
	return str[0];
    }

    switch (str[1]) {
      case 'n':
	return '\n';
      case 't':
	return '\t';
      case 'v':
	return '\v';
      case 'b':
	return '\b';
      case 'r':
	return '\r';
      case 'f':
	return '\f';
      case 'a':
	return '\a';
      case '\\':
	return '\\';
      case '\?':
	return '?';
      case '\'':
	return '\'';
      case '"':
	return '"';
      case 'x':
	{
	    // hex value
            int i;
	    for (i = 2; str[i] != '\0' && isxdigit(str[i]); i++) {
		continue;
	    }
	    char save = str[i];
	    str[i] = '\0';
	    char out = (char)idl_atoi(&str[2], 16);
	    str[i] = save;
	    return out;
	}
	break;
      default:
	// check for octal value
	if (str[1] >= '0' && str[1] <= '7') {
            int i;
	    for (i = 1; str[i] >= '0' && str[i] <= '7'; i++) {
		continue;
	    }
	    char save = str[i];
	    str[i] = '\0';
	    char out = (char)idl_atoi(&str[1], 8);
	    str[i] = save;
	    return out;
	} else {
	  return str[1] - 'a';
	}
	break;
    }
}
int yyvstop[] = {
0,

59,
0,

59,
0,

61,
0,

59,
61,
0,

60,
0,

61,
0,

61,
0,

61,
0,

61,
0,

43,
61,
0,

41,
61,
0,

61,
0,

61,
0,

61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

38,
61,
0,

59,
61,
0,

61,
0,

59,
0,

44,
0,

43,
0,

41,
0,

58,
0,

39,
0,

43,
0,

37,
0,

35,
0,

36,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

32,
38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

59,
0,

53,
0,

45,
0,

45,
0,

57,
0,

39,
0,

40,
0,

42,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

33,
38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

47,
0,

46,
47,
0,

39,
0,

40,
0,

38,
0,

29,
38,
0,

38,
0,

38,
0,

17,
38,
0,

24,
38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

11,
38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

21,
38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

28,
38,
0,

38,
0,

38,
0,

46,
0,

30,
38,
0,

38,
0,

38,
0,

8,
38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

19,
38,
0,

31,
38,
0,

38,
0,

38,
0,

27,
38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

22,
38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

15,
38,
0,

38,
0,

38,
0,

38,
0,

52,
0,

38,
0,

38,
0,

38,
0,

38,
0,

20,
38,
0,

38,
0,

38,
0,

1,
38,
0,

34,
38,
0,

2,
38,
0,

38,
0,

38,
0,

12,
38,
0,

10,
38,
0,

16,
38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

55,
0,

38,
0,

26,
38,
0,

6,
38,
0,

18,
38,
0,

38,
0,

38,
0,

38,
0,

38,
0,

9,
38,
0,

38,
0,

25,
38,
0,

38,
0,

56,
0,

38,
0,

38,
0,

38,
0,

3,
38,
0,

14,
38,
0,

23,
38,
0,

38,
0,

4,
38,
0,

5,
38,
0,

7,
38,
0,

13,
38,
0,

54,
0,

51,
0,

-49,
0,

-50,
0,

51,
0,

-48,
0,

49,
51,
0,

49,
0,

51,
0,

50,
51,
0,

50,
0,

48,
51,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,3,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,4,	1,5,	
0,0,	4,36,	0,0,	34,83,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	1,6,	
4,36,	1,3,	34,83,	87,133,	
1,7,	34,84,	0,0,	39,92,	
1,3,	93,138,	1,8,	9,43,	
1,9,	1,10,	1,11,	0,0,	
9,44,	11,45,	2,34,	11,42,	
42,45,	1,11,	42,42,	1,12,	
0,0,	1,13,	12,50,	1,14,	
13,51,	0,0,	1,15,	14,52,	
0,0,	16,54,	1,15,	1,16,	
1,15,	0,0,	0,0,	103,145,	
11,48,	1,15,	0,0,	42,48,	
2,35,	54,102,	0,0,	0,0,	
2,7,	1,17,	17,55,	134,133,	
0,0,	1,15,	2,8,	55,103,	
2,9,	102,144,	0,0,	0,0,	
1,3,	0,0,	1,18,	1,19,	
1,20,	1,21,	1,22,	1,23,	
59,107,	2,13,	1,24,	2,14,	
11,48,	1,25,	1,26,	42,48,	
1,27,	23,65,	21,61,	1,28,	
1,29,	1,30,	1,31,	1,32,	
1,33,	18,56,	19,57,	20,58,	
21,62,	22,63,	24,66,	25,67,	
26,68,	2,17,	20,59,	30,78,	
6,37,	29,74,	28,72,	22,64,	
29,75,	20,60,	28,73,	31,79,	
6,37,	6,37,	2,18,	2,19,	
2,20,	2,21,	2,22,	2,23,	
29,76,	32,80,	2,24,	29,77,	
56,104,	2,25,	2,26,	33,81,	
2,27,	27,69,	57,105,	2,28,	
2,29,	2,30,	2,31,	2,32,	
2,33,	6,38,	58,106,	6,37,	
27,70,	60,108,	61,109,	33,82,	
62,110,	7,39,	6,37,	27,71,	
63,111,	64,112,	65,113,	6,37,	
6,37,	7,39,	7,0,	66,114,	
67,116,	68,117,	69,118,	6,37,	
66,115,	6,37,	70,119,	71,120,	
72,121,	73,122,	74,123,	75,124,	
6,37,	76,125,	77,126,	78,127,	
6,37,	6,37,	6,37,	79,128,	
80,130,	81,131,	7,39,	6,37,	
7,39,	82,132,	89,135,	90,136,	
91,137,	79,129,	104,146,	7,39,	
105,147,	106,148,	107,149,	6,37,	
7,39,	7,39,	108,150,	108,151,	
109,152,	110,153,	6,37,	100,143,	
7,39,	111,154,	7,39,	112,155,	
113,156,	100,143,	114,157,	115,158,	
116,159,	7,39,	117,160,	118,161,	
119,162,	7,39,	7,39,	7,39,	
121,163,	122,164,	123,165,	124,166,	
7,39,	8,41,	8,42,	8,42,	
8,42,	8,42,	8,42,	8,42,	
8,42,	8,42,	8,42,	100,143,	
7,39,	126,169,	127,170,	128,171,	
7,40,	100,143,	10,45,	7,39,	
10,46,	10,46,	10,46,	10,46,	
10,46,	10,46,	10,46,	10,46,	
10,47,	10,47,	47,47,	47,47,	
47,47,	47,47,	47,47,	47,47,	
47,47,	47,47,	48,99,	129,172,	
48,99,	10,48,	125,167,	48,100,	
48,100,	48,100,	48,100,	48,100,	
48,100,	48,100,	48,100,	48,100,	
48,100,	95,139,	125,168,	130,173,	
131,174,	132,175,	135,177,	136,178,	
10,49,	137,179,	95,140,	95,140,	
95,140,	95,140,	95,140,	95,140,	
95,140,	95,140,	144,182,	146,183,	
147,184,	10,48,	150,185,	151,186,	
15,53,	15,53,	15,53,	15,53,	
15,53,	15,53,	15,53,	15,53,	
15,53,	15,53,	152,187,	153,188,	
155,189,	156,190,	157,191,	158,192,	
10,49,	15,53,	15,53,	15,53,	
15,53,	15,53,	15,53,	15,53,	
15,53,	15,53,	15,53,	15,53,	
15,53,	15,53,	15,53,	15,53,	
15,53,	15,53,	15,53,	15,53,	
15,53,	15,53,	15,53,	15,53,	
15,53,	15,53,	15,53,	160,193,	
161,194,	162,195,	163,196,	15,53,	
164,197,	15,53,	15,53,	15,53,	
15,53,	15,53,	15,53,	15,53,	
15,53,	15,53,	15,53,	15,53,	
15,53,	15,53,	15,53,	15,53,	
15,53,	15,53,	15,53,	15,53,	
15,53,	15,53,	15,53,	15,53,	
15,53,	15,53,	15,53,	35,85,	
35,86,	165,198,	166,199,	167,200,	
46,45,	168,201,	46,46,	46,46,	
46,46,	46,46,	46,46,	46,46,	
46,46,	46,46,	46,47,	46,47,	
40,93,	169,202,	170,203,	171,204,	
172,205,	174,206,	35,87,	175,207,	
40,93,	40,0,	35,84,	46,48,	
99,100,	99,100,	99,100,	99,100,	
99,100,	99,100,	99,100,	99,100,	
99,100,	99,100,	35,88,	35,88,	
35,88,	35,88,	35,88,	35,88,	
35,88,	35,88,	35,88,	35,88,	
177,209,	40,93,	142,98,	40,93,	
178,210,	179,211,	40,94,	181,180,	
142,98,	44,44,	40,93,	46,48,	
183,212,	184,213,	186,214,	40,95,	
40,95,	44,44,	44,96,	187,215,	
188,216,	189,217,	192,218,	40,93,	
193,219,	40,93,	195,220,	196,221,	
197,222,	198,223,	200,224,	201,225,	
40,93,	202,226,	142,98,	203,227,	
40,93,	40,93,	40,93,	205,228,	
142,98,	206,229,	44,44,	40,93,	
44,44,	207,230,	209,231,	35,89,	
211,235,	212,236,	35,90,	44,44,	
213,237,	214,238,	35,91,	40,93,	
44,44,	44,44,	215,239,	217,240,	
218,241,	222,242,	40,93,	223,243,	
44,44,	227,244,	44,44,	228,245,	
229,246,	230,247,	233,249,	235,251,	
236,252,	44,44,	240,253,	241,254,	
242,255,	44,44,	44,44,	44,44,	
243,256,	245,257,	247,258,	250,249,	
44,44,	45,45,	45,45,	45,45,	
45,45,	45,45,	45,45,	45,45,	
45,45,	45,45,	45,45,	252,261,	
44,44,	253,262,	254,263,	258,264,	
268,271,	251,260,	269,272,	44,44,	
270,273,	271,274,	45,97,	45,98,	
272,275,	273,276,	275,278,	276,279,	
278,281,	45,98,	49,101,	49,101,	
49,101,	49,101,	49,101,	49,101,	
49,101,	49,101,	49,101,	49,101,	
251,260,	279,282,	280,280,	281,284,	
282,285,	284,288,	84,84,	49,101,	
49,101,	49,101,	49,101,	49,101,	
49,101,	285,289,	45,97,	45,98,	
287,287,	287,292,	289,294,	297,297,	
297,301,	45,98,	304,305,	0,0,	
304,304,	304,304,	0,0,	280,283,	
0,0,	84,84,	305,306,	305,306,	
304,304,	84,84,	88,86,	291,291,	
291,295,	0,0,	305,306,	49,101,	
49,101,	49,101,	49,101,	49,101,	
49,101,	84,84,	84,84,	84,84,	
84,84,	84,84,	84,84,	84,84,	
84,84,	84,84,	84,84,	97,141,	
88,134,	97,141,	291,291,	288,288,	
97,142,	97,142,	97,142,	97,142,	
97,142,	97,142,	97,142,	97,142,	
97,142,	97,142,	0,0,	302,302,	
88,88,	88,88,	88,88,	88,88,	
88,88,	88,88,	88,88,	88,88,	
88,88,	88,88,	133,133,	140,180,	
288,293,	300,300,	300,303,	306,307,	
306,308,	0,0,	133,133,	133,133,	
140,181,	140,181,	140,181,	140,181,	
140,181,	140,181,	140,181,	140,181,	
307,307,	307,308,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
300,300,	84,91,	302,304,	302,304,	
0,0,	0,0,	0,0,	133,176,	
0,0,	133,133,	302,304,	0,0,	
0,0,	0,0,	0,0,	0,0,	
133,133,	0,0,	306,306,	306,306,	
0,0,	133,133,	133,133,	0,0,	
0,0,	176,176,	306,306,	0,0,	
0,0,	133,133,	0,0,	133,133,	
0,0,	176,176,	176,208,	0,0,	
0,0,	0,0,	133,133,	0,0,	
0,0,	0,0,	133,133,	133,133,	
133,133,	0,0,	0,0,	0,0,	
0,0,	133,133,	141,142,	141,142,	
141,142,	141,142,	141,142,	141,142,	
141,142,	141,142,	141,142,	141,142,	
176,176,	133,133,	210,210,	210,232,	
0,0,	0,0,	0,0,	176,176,	
133,133,	0,0,	0,0,	0,0,	
176,176,	176,176,	0,0,	0,0,	
0,0,	0,0,	231,231,	0,0,	
176,176,	0,0,	176,176,	0,0,	
0,0,	210,233,	231,231,	231,248,	
0,0,	176,176,	0,0,	0,0,	
0,0,	176,176,	176,176,	176,176,	
0,0,	0,0,	0,0,	0,0,	
176,176,	210,234,	210,234,	210,234,	
210,234,	210,234,	210,234,	210,234,	
210,234,	210,234,	210,234,	231,231,	
176,176,	231,231,	0,0,	0,0,	
234,232,	0,0,	0,0,	176,176,	
231,231,	0,0,	0,0,	0,0,	
0,0,	231,231,	231,231,	0,0,	
0,0,	0,0,	0,0,	249,249,	
0,0,	231,231,	0,0,	231,231,	
0,0,	0,0,	234,250,	249,249,	
249,249,	0,0,	231,231,	0,0,	
0,0,	0,0,	231,231,	231,231,	
231,231,	0,0,	0,0,	0,0,	
0,0,	231,231,	234,234,	234,234,	
234,234,	234,234,	234,234,	234,234,	
234,234,	234,234,	234,234,	234,234,	
249,259,	231,231,	249,249,	259,259,	
0,0,	0,0,	0,0,	0,0,	
231,231,	249,249,	0,0,	259,259,	
259,265,	0,0,	249,249,	249,249,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	249,249,	0,0,	
249,249,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	249,249,	
0,0,	0,0,	0,0,	249,249,	
249,249,	249,249,	259,259,	0,0,	
0,0,	0,0,	249,249,	0,0,	
260,266,	259,259,	0,0,	0,0,	
0,0,	0,0,	259,259,	259,259,	
260,260,	260,267,	249,249,	0,0,	
0,0,	0,0,	259,259,	0,0,	
259,259,	249,249,	0,0,	0,0,	
0,0,	0,0,	0,0,	259,259,	
0,0,	0,0,	0,0,	259,259,	
259,259,	259,259,	0,0,	0,0,	
0,0,	260,266,	259,259,	260,266,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	260,266,	0,0,	
0,0,	0,0,	259,259,	260,266,	
260,266,	0,0,	266,266,	0,0,	
0,0,	259,259,	0,0,	260,266,	
0,0,	260,266,	266,266,	266,267,	
0,0,	0,0,	0,0,	0,0,	
260,266,	0,0,	274,274,	0,0,	
260,266,	260,266,	260,266,	0,0,	
260,268,	0,0,	0,0,	260,266,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	266,266,	
0,0,	266,266,	0,0,	260,266,	
0,0,	0,0,	0,0,	277,280,	
266,266,	0,0,	260,266,	0,0,	
0,0,	266,266,	266,266,	0,0,	
0,0,	294,294,	0,0,	0,0,	
0,0,	266,266,	0,0,	266,266,	
0,0,	0,0,	0,0,	260,269,	
0,0,	0,0,	266,266,	274,277,	
0,0,	260,270,	266,266,	266,266,	
266,266,	0,0,	274,277,	0,0,	
0,0,	266,266,	274,277,	274,277,	
274,277,	0,0,	277,277,	277,277,	
0,0,	274,277,	283,283,	0,0,	
0,0,	266,266,	277,277,	0,0,	
277,277,	0,0,	283,283,	283,286,	
266,266,	274,277,	0,0,	277,277,	
0,0,	0,0,	294,298,	277,277,	
277,277,	277,277,	0,0,	0,0,	
0,0,	294,298,	277,277,	0,0,	
0,0,	294,298,	294,298,	294,298,	
0,0,	0,0,	0,0,	283,287,	
294,298,	283,283,	277,277,	0,0,	
0,0,	0,0,	0,0,	286,290,	
283,283,	277,277,	293,293,	0,0,	
294,298,	283,283,	283,283,	286,290,	
286,290,	0,0,	293,293,	293,296,	
0,0,	283,283,	0,0,	283,283,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	283,283,	0,0,	
0,0,	0,0,	283,283,	283,283,	
283,283,	0,0,	0,0,	0,0,	
286,291,	283,283,	286,290,	293,297,	
0,0,	293,293,	0,0,	0,0,	
0,0,	286,290,	0,0,	0,0,	
293,293,	283,283,	286,290,	286,290,	
0,0,	293,293,	293,293,	0,0,	
283,283,	0,0,	286,290,	0,0,	
286,290,	293,293,	0,0,	293,293,	
0,0,	0,0,	0,0,	286,290,	
0,0,	0,0,	293,293,	286,290,	
286,290,	286,290,	293,293,	293,293,	
293,293,	296,299,	286,290,	0,0,	
0,0,	293,293,	298,302,	0,0,	
0,0,	296,299,	296,299,	0,0,	
0,0,	0,0,	286,290,	0,0,	
0,0,	293,293,	0,0,	0,0,	
0,0,	286,290,	0,0,	0,0,	
293,293,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	296,300,	0,0,	
296,299,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	296,299,	
0,0,	298,298,	298,298,	0,0,	
296,299,	296,299,	0,0,	0,0,	
0,0,	298,298,	0,0,	298,298,	
296,299,	0,0,	296,299,	0,0,	
0,0,	0,0,	298,298,	0,0,	
0,0,	296,299,	298,298,	298,298,	
298,298,	296,299,	296,299,	296,299,	
0,0,	298,298,	0,0,	0,0,	
296,299,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	298,298,	0,0,	0,0,	
296,299,	0,0,	0,0,	0,0,	
298,298,	0,0,	0,0,	296,299,	
0,0};

#ifdef __DECCXX
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
 	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
#endif

struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		yyvstop+1,
yycrank+-45,	yysvec+1,	yyvstop+3,
yycrank+0,	0,		yyvstop+5,
yycrank+4,	0,		yyvstop+7,
yycrank+0,	0,		yyvstop+10,
yycrank+-131,	0,		yyvstop+12,
yycrank+-172,	0,		yyvstop+14,
yycrank+201,	0,		yyvstop+16,
yycrank+5,	0,		yyvstop+18,
yycrank+220,	0,		yyvstop+20,
yycrank+7,	yysvec+8,	yyvstop+23,
yycrank+4,	0,		yyvstop+26,
yycrank+4,	0,		yyvstop+28,
yycrank+5,	0,		yyvstop+30,
yycrank+276,	0,		yyvstop+32,
yycrank+4,	yysvec+15,	yyvstop+35,
yycrank+4,	yysvec+15,	yyvstop+38,
yycrank+5,	yysvec+15,	yyvstop+41,
yycrank+11,	yysvec+15,	yyvstop+44,
yycrank+26,	yysvec+15,	yyvstop+47,
yycrank+13,	yysvec+15,	yyvstop+50,
yycrank+15,	yysvec+15,	yyvstop+53,
yycrank+5,	yysvec+15,	yyvstop+56,
yycrank+16,	yysvec+15,	yyvstop+59,
yycrank+16,	yysvec+15,	yyvstop+62,
yycrank+17,	yysvec+15,	yyvstop+65,
yycrank+58,	yysvec+15,	yyvstop+68,
yycrank+37,	yysvec+15,	yyvstop+71,
yycrank+32,	yysvec+15,	yyvstop+74,
yycrank+10,	yysvec+15,	yyvstop+77,
yycrank+29,	yysvec+15,	yyvstop+80,
yycrank+38,	yysvec+15,	yyvstop+83,
yycrank+56,	yysvec+15,	yyvstop+86,
yycrank+6,	0,		yyvstop+89,
yycrank+390,	0,		yyvstop+92,
yycrank+0,	yysvec+4,	yyvstop+94,
yycrank+0,	yysvec+6,	0,	
yycrank+0,	0,		yyvstop+96,
yycrank+4,	0,		0,	
yycrank+-415,	0,		0,	
yycrank+0,	yysvec+10,	yyvstop+98,
yycrank+10,	yysvec+8,	yyvstop+100,
yycrank+0,	0,		yyvstop+102,
yycrank+-456,	0,		0,	
yycrank+485,	0,		yyvstop+104,
yycrank+358,	0,		yyvstop+106,
yycrank+230,	yysvec+46,	0,	
yycrank+243,	0,		0,	
yycrank+514,	0,		0,	
yycrank+0,	0,		yyvstop+108,
yycrank+0,	0,		yyvstop+110,
yycrank+0,	0,		yyvstop+112,
yycrank+0,	yysvec+15,	yyvstop+114,
yycrank+5,	yysvec+15,	yyvstop+116,
yycrank+6,	yysvec+15,	yyvstop+118,
yycrank+36,	yysvec+15,	yyvstop+120,
yycrank+47,	yysvec+15,	yyvstop+122,
yycrank+51,	yysvec+15,	yyvstop+124,
yycrank+7,	yysvec+15,	yyvstop+126,
yycrank+59,	yysvec+15,	yyvstop+128,
yycrank+68,	yysvec+15,	yyvstop+130,
yycrank+55,	yysvec+15,	yyvstop+132,
yycrank+59,	yysvec+15,	yyvstop+134,
yycrank+78,	yysvec+15,	yyvstop+136,
yycrank+67,	yysvec+15,	yyvstop+138,
yycrank+72,	yysvec+15,	yyvstop+140,
yycrank+74,	yysvec+15,	yyvstop+143,
yycrank+85,	yysvec+15,	yyvstop+145,
yycrank+70,	yysvec+15,	yyvstop+147,
yycrank+89,	yysvec+15,	yyvstop+149,
yycrank+75,	yysvec+15,	yyvstop+151,
yycrank+87,	yysvec+15,	yyvstop+153,
yycrank+96,	yysvec+15,	yyvstop+155,
yycrank+81,	yysvec+15,	yyvstop+157,
yycrank+84,	yysvec+15,	yyvstop+159,
yycrank+83,	yysvec+15,	yyvstop+161,
yycrank+93,	yysvec+15,	yyvstop+163,
yycrank+87,	yysvec+15,	yyvstop+165,
yycrank+98,	yysvec+15,	yyvstop+167,
yycrank+99,	yysvec+15,	yyvstop+169,
yycrank+101,	yysvec+15,	yyvstop+171,
yycrank+93,	yysvec+15,	yyvstop+173,
yycrank+0,	yysvec+34,	yyvstop+175,
yycrank+569,	0,		0,	
yycrank+0,	yysvec+35,	0,	
yycrank+0,	0,		yyvstop+177,
yycrank+5,	yysvec+35,	0,	
yycrank+596,	yysvec+84,	0,	
yycrank+110,	0,		0,	
yycrank+106,	0,		0,	
yycrank+98,	0,		0,	
yycrank+0,	0,		yyvstop+179,
yycrank+6,	0,		0,	
yycrank+0,	yysvec+93,	yyvstop+181,
yycrank+262,	0,		0,	
yycrank+0,	0,		yyvstop+183,
yycrank+584,	0,		0,	
yycrank+0,	0,		yyvstop+185,
yycrank+380,	0,		0,	
yycrank+157,	yysvec+99,	yyvstop+187,
yycrank+0,	yysvec+49,	yyvstop+189,
yycrank+10,	yysvec+15,	yyvstop+191,
yycrank+6,	yysvec+15,	yyvstop+193,
yycrank+100,	yysvec+15,	yyvstop+195,
yycrank+108,	yysvec+15,	yyvstop+197,
yycrank+116,	yysvec+15,	yyvstop+199,
yycrank+104,	yysvec+15,	yyvstop+201,
yycrank+107,	yysvec+15,	yyvstop+203,
yycrank+127,	yysvec+15,	yyvstop+205,
yycrank+127,	yysvec+15,	yyvstop+207,
yycrank+120,	yysvec+15,	yyvstop+209,
yycrank+130,	yysvec+15,	yyvstop+211,
yycrank+135,	yysvec+15,	yyvstop+213,
yycrank+117,	yysvec+15,	yyvstop+215,
yycrank+134,	yysvec+15,	yyvstop+217,
yycrank+133,	yysvec+15,	yyvstop+219,
yycrank+121,	yysvec+15,	yyvstop+221,
yycrank+138,	yysvec+15,	yyvstop+223,
yycrank+121,	yysvec+15,	yyvstop+225,
yycrank+0,	yysvec+15,	yyvstop+227,
yycrank+129,	yysvec+15,	yyvstop+230,
yycrank+145,	yysvec+15,	yyvstop+232,
yycrank+129,	yysvec+15,	yyvstop+234,
yycrank+133,	yysvec+15,	yyvstop+236,
yycrank+185,	yysvec+15,	yyvstop+238,
yycrank+145,	yysvec+15,	yyvstop+240,
yycrank+161,	yysvec+15,	yyvstop+242,
yycrank+152,	yysvec+15,	yyvstop+244,
yycrank+182,	yysvec+15,	yyvstop+246,
yycrank+203,	yysvec+15,	yyvstop+248,
yycrank+207,	yysvec+15,	yyvstop+250,
yycrank+191,	yysvec+15,	yyvstop+252,
yycrank+-653,	0,		0,	
yycrank+53,	yysvec+84,	0,	
yycrank+205,	0,		0,	
yycrank+197,	0,		0,	
yycrank+212,	0,		0,	
yycrank+0,	0,		yyvstop+254,
yycrank+0,	0,		yyvstop+256,
yycrank+616,	0,		0,	
yycrank+682,	0,		0,	
yycrank+380,	yysvec+141,	yyvstop+259,
yycrank+0,	0,		yyvstop+261,
yycrank+249,	yysvec+15,	yyvstop+263,
yycrank+0,	yysvec+15,	yyvstop+265,
yycrank+214,	yysvec+15,	yyvstop+268,
yycrank+219,	yysvec+15,	yyvstop+270,
yycrank+0,	yysvec+15,	yyvstop+272,
yycrank+0,	yysvec+15,	yyvstop+275,
yycrank+206,	yysvec+15,	yyvstop+278,
yycrank+222,	yysvec+15,	yyvstop+280,
yycrank+217,	yysvec+15,	yyvstop+282,
yycrank+227,	yysvec+15,	yyvstop+284,
yycrank+0,	yysvec+15,	yyvstop+286,
yycrank+224,	yysvec+15,	yyvstop+289,
yycrank+221,	yysvec+15,	yyvstop+291,
yycrank+222,	yysvec+15,	yyvstop+293,
yycrank+225,	yysvec+15,	yyvstop+295,
yycrank+0,	yysvec+15,	yyvstop+297,
yycrank+259,	yysvec+15,	yyvstop+300,
yycrank+252,	yysvec+15,	yyvstop+302,
yycrank+272,	yysvec+15,	yyvstop+304,
yycrank+269,	yysvec+15,	yyvstop+306,
yycrank+261,	yysvec+15,	yyvstop+308,
yycrank+300,	yysvec+15,	yyvstop+310,
yycrank+286,	yysvec+15,	yyvstop+312,
yycrank+293,	yysvec+15,	yyvstop+314,
yycrank+306,	yysvec+15,	yyvstop+316,
yycrank+318,	yysvec+15,	yyvstop+318,
yycrank+318,	yysvec+15,	yyvstop+320,
yycrank+309,	yysvec+15,	yyvstop+322,
yycrank+317,	yysvec+15,	yyvstop+324,
yycrank+0,	yysvec+15,	yyvstop+326,
yycrank+307,	yysvec+15,	yyvstop+329,
yycrank+318,	yysvec+15,	yyvstop+331,
yycrank+-704,	yysvec+133,	0,	
yycrank+338,	0,		0,	
yycrank+351,	0,		0,	
yycrank+350,	0,		0,	
yycrank+0,	0,		yyvstop+333,
yycrank+416,	0,		0,	
yycrank+0,	yysvec+15,	yyvstop+335,
yycrank+362,	yysvec+15,	yyvstop+338,
yycrank+364,	yysvec+15,	yyvstop+340,
yycrank+0,	yysvec+15,	yyvstop+342,
yycrank+342,	yysvec+15,	yyvstop+345,
yycrank+359,	yysvec+15,	yyvstop+347,
yycrank+367,	yysvec+15,	yyvstop+349,
yycrank+353,	yysvec+15,	yyvstop+351,
yycrank+0,	yysvec+15,	yyvstop+353,
yycrank+0,	yysvec+15,	yyvstop+356,
yycrank+368,	yysvec+15,	yyvstop+359,
yycrank+371,	yysvec+15,	yyvstop+361,
yycrank+0,	yysvec+15,	yyvstop+363,
yycrank+353,	yysvec+15,	yyvstop+366,
yycrank+360,	yysvec+15,	yyvstop+368,
yycrank+366,	yysvec+15,	yyvstop+370,
yycrank+367,	yysvec+15,	yyvstop+372,
yycrank+0,	yysvec+15,	yyvstop+374,
yycrank+375,	yysvec+15,	yyvstop+377,
yycrank+363,	yysvec+15,	yyvstop+379,
yycrank+377,	yysvec+15,	yyvstop+381,
yycrank+382,	yysvec+15,	yyvstop+383,
yycrank+0,	yysvec+15,	yyvstop+385,
yycrank+377,	yysvec+15,	yyvstop+388,
yycrank+394,	yysvec+15,	yyvstop+390,
yycrank+383,	yysvec+15,	yyvstop+392,
yycrank+0,	0,		yyvstop+394,
yycrank+378,	0,		0,	
yycrank+733,	0,		0,	
yycrank+387,	0,		0,	
yycrank+380,	yysvec+15,	yyvstop+396,
yycrank+390,	yysvec+15,	yyvstop+398,
yycrank+385,	yysvec+15,	yyvstop+400,
yycrank+390,	yysvec+15,	yyvstop+402,
yycrank+0,	yysvec+15,	yyvstop+404,
yycrank+402,	yysvec+15,	yyvstop+407,
yycrank+411,	yysvec+15,	yyvstop+409,
yycrank+0,	yysvec+15,	yyvstop+411,
yycrank+0,	yysvec+15,	yyvstop+414,
yycrank+0,	yysvec+15,	yyvstop+417,
yycrank+401,	yysvec+15,	yyvstop+420,
yycrank+412,	yysvec+15,	yyvstop+422,
yycrank+0,	yysvec+15,	yyvstop+424,
yycrank+0,	yysvec+15,	yyvstop+427,
yycrank+0,	yysvec+15,	yyvstop+430,
yycrank+411,	yysvec+15,	yyvstop+433,
yycrank+414,	yysvec+15,	yyvstop+435,
yycrank+400,	yysvec+15,	yyvstop+437,
yycrank+414,	yysvec+15,	yyvstop+439,
yycrank+-757,	0,		0,	
yycrank+0,	0,		yyvstop+441,
yycrank+484,	yysvec+210,	0,	
yycrank+786,	0,		0,	
yycrank+422,	0,		0,	
yycrank+404,	yysvec+15,	yyvstop+443,
yycrank+0,	yysvec+15,	yyvstop+445,
yycrank+0,	yysvec+15,	yyvstop+448,
yycrank+0,	yysvec+15,	yyvstop+451,
yycrank+411,	yysvec+15,	yyvstop+454,
yycrank+424,	yysvec+15,	yyvstop+456,
yycrank+403,	yysvec+15,	yyvstop+458,
yycrank+427,	yysvec+15,	yyvstop+460,
yycrank+0,	yysvec+15,	yyvstop+462,
yycrank+429,	yysvec+15,	yyvstop+465,
yycrank+0,	yysvec+15,	yyvstop+467,
yycrank+435,	yysvec+15,	yyvstop+470,
yycrank+0,	0,		yyvstop+472,
yycrank+-810,	0,		0,	
yycrank+497,	0,		0,	
yycrank+540,	0,		0,	
yycrank+442,	yysvec+15,	yyvstop+474,
yycrank+435,	yysvec+15,	yyvstop+476,
yycrank+445,	yysvec+15,	yyvstop+478,
yycrank+0,	yysvec+15,	yyvstop+480,
yycrank+0,	yysvec+15,	yyvstop+483,
yycrank+0,	yysvec+15,	yyvstop+486,
yycrank+431,	yysvec+15,	yyvstop+489,
yycrank+-846,	yysvec+249,	0,	
yycrank+-887,	0,		0,	
yycrank+0,	yysvec+15,	yyvstop+491,
yycrank+0,	yysvec+15,	yyvstop+494,
yycrank+0,	yysvec+15,	yyvstop+497,
yycrank+0,	yysvec+15,	yyvstop+500,
yycrank+0,	0,		yyvstop+503,
yycrank+-937,	0,		0,	
yycrank+0,	0,		yyvstop+505,
yycrank+-480,	yysvec+266,	0,	
yycrank+-436,	yysvec+266,	0,	
yycrank+-451,	yysvec+266,	0,	
yycrank+-544,	yysvec+266,	yyvstop+507,
yycrank+-455,	yysvec+266,	0,	
yycrank+-443,	yysvec+266,	0,	
yycrank+-945,	yysvec+266,	0,	
yycrank+-456,	yysvec+266,	0,	
yycrank+-444,	yysvec+266,	0,	
yycrank+-970,	yysvec+266,	0,	
yycrank+-455,	yysvec+266,	0,	
yycrank+-468,	yysvec+266,	0,	
yycrank+-565,	yysvec+266,	0,	
yycrank+-455,	yysvec+266,	0,	
yycrank+-465,	yysvec+266,	0,	
yycrank+-1021,	0,		0,	
yycrank+-568,	yysvec+266,	yyvstop+509,
yycrank+-475,	yysvec+266,	0,	
yycrank+-1062,	0,		yyvstop+511,
yycrank+-579,	yysvec+266,	0,	
yycrank+-622,	yysvec+266,	0,	
yycrank+-581,	yysvec+266,	yyvstop+513,
yycrank+0,	yysvec+286,	0,	
yycrank+598,	0,		0,	
yycrank+0,	0,		yyvstop+515,
yycrank+-1065,	0,		0,	
yycrank+-980,	yysvec+266,	0,	
yycrank+0,	0,		yyvstop+518,
yycrank+-1136,	0,		yyvstop+520,
yycrank+-582,	yysvec+266,	0,	
yycrank+-1133,	yysvec+266,	0,	
yycrank+0,	yysvec+296,	0,	
yycrank+648,	0,		0,	
yycrank+0,	0,		yyvstop+522,
yycrank+-634,	yysvec+266,	0,	
yycrank+0,	0,		yyvstop+525,
yycrank+-548,	yysvec+266,	0,	
yycrank+-554,	yysvec+266,	0,	
yycrank+-650,	yysvec+266,	0,	
yycrank+-663,	yysvec+266,	0,	
yycrank+0,	0,		yyvstop+527,
0,	0,	0};
struct yywork *yytop = yycrank+1231;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
  0,   1,   1,   1,   1,   1,   1,   1, 
  1,   9,  10,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  9,   1,  34,   1,  36,   1,   1,   1, 
  1,   1,   1,  43,   1,  43,   1,   1, 
 48,  49,  49,  49,  49,  49,  49,  49, 
 56,  56,  58,   1,   1,   1,   1,   1, 
  1,  65,  65,  65,  65,  69,  70,  71, 
 71,  71,  71,  71,  76,  71,  71,  71, 
 71,  71,  71,  71,  71,  71,  71,  71, 
 88,  71,  71,   1,   1,   1,   1,  95, 
  1,  65,  65,  65,  65,  69,  70,  71, 
 71,  71,  71,  71,  76,  71,  71,  71, 
 71,  71,  71,  71,  71,  71,  71,  71, 
 88,  71,  71,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
1,1,1,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	Copyright (c) 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
#if defined(__cplusplus) || defined(__STDC__)
int yylook(void)
#else
yylook()
#endif
{
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
#ifndef __cplusplus
			*yylastch++ = yych = input();
#else
			*yylastch++ = yych = lex_input();
#endif
			if(yylastch > &yytext[YYLMAX]) {
				fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
				exit(1);
			}
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
#ifndef __cplusplus
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
#else
		yyprevious = yytext[0] = lex_input();
		if (yyprevious>0)
			lex_output(yyprevious);
#endif
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
#if defined(__cplusplus) || defined(__STDC__)
int yyback(int *p, int m)
#else
yyback(p, m)
	int *p;
#endif
{
	if (p==0) return(0);
	while (*p) {
		if (*p++ == m)
			return(1);
	}
	return(0);
}
	/* the following are only used in the lex library */
#if defined(__cplusplus) || defined(__STDC__)
int yyinput(void)
#else
yyinput()
#endif
{
#ifndef __cplusplus
	return(input());
#else
	return(lex_input());
#endif
	}
#if defined(__cplusplus) || defined(__STDC__)
void yyoutput(int c)
#else
yyoutput(c)
  int c; 
#endif
{
#ifndef __cplusplus
	output(c);
#else
	lex_output(c);
#endif
	}
#if defined(__cplusplus) || defined(__STDC__)
void yyunput(int c)
#else
yyunput(c)
   int c; 
#endif
{
	unput(c);
	}
#ifdef __VMS
# line 2691 "lex_yy.cc"
#endif
