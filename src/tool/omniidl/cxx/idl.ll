%{ // -*- c++ -*-
//                          Package   : omniidl
// idl.ll                   Created on: 1999/10/05
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 1999 AT&T Laboratories Cambridge
//
//  This file is part of omniidl.
//
//  omniidl is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//
// Description:
//   
//   Flex lexical analyser

// $Id$
// $Log$
// Revision 1.7.2.5  2000/06/08 14:36:19  dpg1
// Comments and pragmas are now objects rather than plain strings, so
// they can have file,line associated with them.
//
// Revision 1.7.2.4  2000/06/05 18:12:25  dpg1
// Support for __omni_pragma
//
// Revision 1.7.2.3  2000/04/26 09:55:48  dpg1
// cccp escapes characters like \ in line directives, so
// parseLineDirective() needs to use escapedStringToString().
//
// Revision 1.7.2.2  2000/03/06 15:03:49  dpg1
// Minor bug fixes to omniidl. New -nf and -k flags.
//
// Revision 1.7.2.1  2000/02/16 16:30:54  dpg1
// #pragma ID was misspelled #pragma id. Oops.
//
// Revision 1.7  2000/02/04 12:17:10  dpg1
// Support for VMS.
//
// Revision 1.6  1999/12/28 17:44:01  dpg1
// Stupid string allocation bug.
//
// Revision 1.5  1999/11/19 16:03:40  dpg1
// Flex scanner now accepts \r as whitespace.
//
// Revision 1.4  1999/11/05 10:36:42  dpg1
// Sorted out isatty() problems.
//
// Revision 1.3  1999/11/02 17:07:28  dpg1
// Changes to compile on Solaris.
//
// Revision 1.2  1999/11/01 10:05:00  dpg1
// New file attribute to AST.
//
// Revision 1.1  1999/10/27 14:06:00  dpg1
// *** empty log message ***
//

#define YY_NEVER_INTERACTIVE 1

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <idlerr.h>
#include <idlutil.h>
#include <idlrepoId.h>
#include <idlscope.h>
#include <idlexpr.h>
#include <idlast.h>

#ifndef __VMS
#include <y.tab.h>
#else
#include <y_tab.h>
#endif

char*          currentFile;
_CORBA_Boolean mainFile  = 1; // Are we processing the main file
int            nestDepth = 0; // #include nesting depth

char octalToChar(char* s);
char hexToChar(char* s);
char escapeToChar(char* s);
_CORBA_UShort octalToWChar(char* s);
_CORBA_UShort hexToWChar(char* s);
_CORBA_UShort escapeToWChar(char* s);
char* escapedStringToString(char* s);
_CORBA_UShort* escapedStringToWString(char* s);
void parseLineDirective(char* s);
int fixed(char* s);

%}

%option noyywrap
%option yylineno

%x comment
%s known_pragma
%x unknown_pragma
%x omni_pragma

WS          [ \t\v\n\f\r]
SPACE       [ \t]

ASCIILETTER [a-zA-Z]
IDCHAR      [a-zA-Z0-9_]
IDENT       ({ASCIILETTER}{IDCHAR}*)

DECDIGIT    [0-9]
DECSTART    [1-9]
OCTSTART    0
OCTDIGIT    [0-7]
HEXSTART    (0[xX])
HEXDIGIT    [0-9a-fA-F]

EXPONENT    ([Ee][+-]?{DECDIGIT}+)

STR         (\"([^\"]|\\\")*\")

%%

abstract        return ABSTRACT;
any             return ANY;
attribute       return ATTRIBUTE;
boolean         return BOOLEAN;
case            return CASE;
char            return CHAR;
const           return CONST;
context         return CONTEXT;
custom          return CUSTOM;
default         return DEFAULT;
double          return DOUBLE;
enum            return ENUM;
exception       return EXCEPTION;
factory         return FACTORY;
FALSE           return FALSE_;
fixed           return FIXED;
float           return FLOAT;
in              return IN;
inout           return INOUT;
interface       return INTERFACE;
long            return LONG;
module          return MODULE;
native          return NATIVE;
Object          return OBJECT;
octet           return OCTET;
oneway          return ONEWAY;
out             return OUT;
private         return PRIVATE;
public          return PUBLIC;
raises          return RAISES;
readonly        return READONLY;
sequence        return SEQUENCE;
short           return SHORT;
string          return STRING;
struct          return STRUCT;
supports        return SUPPORTS;
switch          return SWITCH;
TRUE            return TRUE_;
truncatable     return TRUNCATABLE;
typedef         return TYPEDEF;
union           return UNION;
unsigned        return UNSIGNED;
ValueBase       return VALUEBASE;
valuetype       return VALUETYPE;
void            return VOID;
wchar           return WCHAR;
wstring         return WSTRING;

"<<"            return LEFT_SHIFT;
">>"            return RIGHT_SHIFT;
"::"            return SCOPE_DELIM;

{IDENT} {
  yylval.id_val = idl_strdup(yytext);
  return IDENTIFIER;
}

_{IDENT} {
  yylval.id_val = idl_strdup(yytext);
  return IDENTIFIER;
}

{DECSTART}{DECDIGIT}* {
  errno = 0;
#ifdef HAS_LongLong
  sscanf(yytext, "%Lu", &yylval.int_literal_val);
#else
  yylval.int_literal_val = strtoul(yytext, 0, 10);
#endif
  if (errno == ERANGE) {
    IdlError(currentFile, yylineno,
	     "Integer literal `%s' is too big", yytext);
  }
  return INTEGER_LITERAL;
}

{OCTSTART}{OCTDIGIT}+ {
  errno = 0;
#ifdef HAS_LongLong
  sscanf(yytext, "%Lo", &yylval.int_literal_val);
#else
  yylval.int_literal_val = strtoul(yytext, 0, 8);
#endif
  if (errno == ERANGE) {
    IdlError(currentFile, yylineno,
	     "Integer literal `%s' is too big", yytext);
  }
  return INTEGER_LITERAL;
}

{HEXSTART}{HEXDIGIT}+ {
  errno = 0;
#ifdef HAS_LongLong
  int r = sscanf(yytext, "%Lx", &yylval.int_literal_val);
#else
  yylval.int_literal_val = strtoul(yytext, 0, 16);
#endif
  if (errno == ERANGE) {
    IdlError(currentFile, yylineno,
	     "Integer literal `%s' is too big", yytext);
  }
  return INTEGER_LITERAL;
}

"0" {
  yylval.int_literal_val = 0;
  return INTEGER_LITERAL;
}

"'"."'" {
  yylval.char_val = yytext[1];
  return CHARACTER_LITERAL;
}

"'"\\([0-7]{1,3})"'" {
  yytext[yyleng-1] = '\0';
  yylval.char_val = octalToChar(yytext + 1);
  return CHARACTER_LITERAL;
}

"'"\\x({HEXDIGIT}{1,2})"'" {
  yytext[yyleng-1] = '\0';
  yylval.char_val = hexToChar(yytext + 1);
  return CHARACTER_LITERAL;
}

"'"\\u({HEXDIGIT}{1,4})"'" {
  IdlError(currentFile, yylineno,
	   "\\u may only be used in wide characters and strings");
  yylval.char_val = 0;
  return CHARACTER_LITERAL;
}

"'"\\."'" {
  yytext[yyleng-1] = '\0';
  yylval.char_val = escapeToChar(yytext + 1);
  return CHARACTER_LITERAL;
}

L"'"."'" {
  yylval.wchar_val = yytext[2];
  return WIDE_CHARACTER_LITERAL;
}

L"'"\\([0-7]{1,3})"'" {
  yytext[yyleng-1] = '\0';
  yylval.wchar_val = octalToWChar(yytext + 2);
  return WIDE_CHARACTER_LITERAL;
}

L"'"\\x({HEXDIGIT}{1,2})"'" {
  yytext[yyleng-1] = '\0';
  yylval.wchar_val = hexToWChar(yytext + 2);
  return WIDE_CHARACTER_LITERAL;
}

L"'"\\u({HEXDIGIT}{1,4})"'" {
  yytext[yyleng-1] = '\0';
  yylval.wchar_val = hexToWChar(yytext + 2);
  return WIDE_CHARACTER_LITERAL;
}

L"'"\\."'" {
  yytext[yyleng-1] = '\0';
  yylval.wchar_val = escapeToWChar(yytext + 2);
  return WIDE_CHARACTER_LITERAL;
}

<INITIAL>{DECDIGIT}+{EXPONENT} {
  // Only deal with floats in INITIAL state, so version pragmas don't
  // get interpreted as floats.
  yylval.float_literal_val = strtod(yytext, 0);
  return FLOATING_PT_LITERAL;
}

<INITIAL>{DECDIGIT}*"."{DECDIGIT}+{EXPONENT}? {
  yylval.float_literal_val = strtod(yytext, 0);
  return FLOATING_PT_LITERAL;
}

<INITIAL>{DECDIGIT}+"."{DECDIGIT}*{EXPONENT}? {
  yylval.float_literal_val = strtod(yytext, 0);
  return FLOATING_PT_LITERAL;
}

{STR} {
  yytext[yyleng-1] = '\0';
  yylval.string_val = escapedStringToString(yytext + 1);
  return STRING_LITERAL;
}

L{STR} {
  yytext[yyleng-1] = '\0';
  yylval.wstring_val = escapedStringToWString(yytext + 1);
  return WIDE_STRING_LITERAL;
}

{DECDIGIT}+"."{DECDIGIT}*[dD] {
  yylval.fixed_val = fixed(yytext);
  return FIXED_PT_LITERAL;
}

{DECDIGIT}*"."{DECDIGIT}+[dD] {
  yylval.fixed_val = fixed(yytext);
  return FIXED_PT_LITERAL;
}

{DECDIGIT}+[dD] {
  yylval.fixed_val = fixed(yytext);
  return FIXED_PT_LITERAL;
}

"//".*\n { Comment::add(yytext, currentFile, yylineno); }

"/*" {
  Comment::add(yytext, currentFile, yylineno);
  BEGIN(comment);
}

<comment>[^*\n]*      { Comment::append(yytext); }
<comment>"*"+[^*/\n]* { Comment::append(yytext); }
<comment>\n           { Comment::append(yytext); }
<comment>"*"+"/"      { Comment::append(yytext); BEGIN(INITIAL); }

"__omni_pragma"{WS}+ {
  BEGIN(omni_pragma);
  return PRAGMA;
}

<omni_pragma>{WS}+"__omni_endpragma" {
  BEGIN(INITIAL);
  return END_PRAGMA;
}

<omni_pragma>[^ \t\v\n\f\r]+ {
  yylval.string_val = idl_strdup(yytext);
  return UNKNOWN_PRAGMA_BODY;
}
<omni_pragma>{WS}+ {
  yylval.string_val = idl_strdup(yytext);
  return UNKNOWN_PRAGMA_BODY;
}


^{SPACE}*#{SPACE}*pragma{SPACE}*prefix{SPACE}* {
  BEGIN(known_pragma);
  return PRAGMA_PREFIX;
}

^{SPACE}*#{SPACE}*pragma{SPACE}*ID{SPACE}* {
  BEGIN(known_pragma);
  return PRAGMA_ID;
}

^{SPACE}*#{SPACE}*pragma{SPACE}*version{SPACE}* {
  BEGIN(known_pragma);
  return PRAGMA_VERSION;
}

<known_pragma>{SPACE}+ { /* Eat spaces and tabs */ }

^{SPACE}*#{SPACE}*pragma{SPACE}* {
  BEGIN(unknown_pragma);
  return PRAGMA;
}

<unknown_pragma>([^\\\n]|(\\[^\n]))+ {
  yylval.string_val = idl_strdup(yytext);
  return UNKNOWN_PRAGMA_BODY;
}

<INITIAL,known_pragma,unknown_pragma>\\\n {
  /* Continue line if it ends with \ */
}

<known_pragma,unknown_pragma>\n {
  BEGIN(INITIAL);
  return END_PRAGMA;
}

^{SPACE}*#{SPACE}*{DECDIGIT}+{SPACE}+{STR}{SPACE}+{DECDIGIT}*{SPACE}*\n {
  parseLineDirective(yytext);
}

^{SPACE}*#{SPACE}*{DECDIGIT}+{SPACE}+{STR}{SPACE}*\n {
  parseLineDirective(yytext);
}

^{SPACE}*#{SPACE}*{DECDIGIT}{SPACE}*\n {
  parseLineDirective(yytext);
}

<INITIAL>{WS} { /* Eat white space */ }

. {
  return yytext[0];
}

%%

char octalToChar(char* s) {
  unsigned long ret = strtoul(s+1, 0, 8);

  if (ret > 255) {
    IdlError(currentFile, yylineno, "Octal character value `%s' too big", s);
  }

  return ret;
}

char hexToChar(char* s)  {
  unsigned long ret = strtoul(s+2, 0, 16);

  return ret;
}

char escapeToChar(char* s) {
  switch (s[1]) {
  case 'n':  return '\n';
  case 't':  return '\t';
  case 'v':  return '\v';
  case 'b':  return '\b';
  case 'r':  return '\r';
  case 'f':  return '\f';
  case 'a':  return '\a';
  case '\\': return '\\';
  case '?':  return '?';
  case '\'': return '\'';
  case '\"': return '\"';
  }
  IdlWarning(currentFile, yylineno,
	     "Behaviour for escape sequence `%s' is undefined by IDL; "
	     "using `%c'", s, s[1]);
  return s[1];
}

_CORBA_UShort octalToWChar(char* s) {
  unsigned long ret = strtoul(s+1, 0, 8);

  if (ret > 255) {
    IdlError(currentFile, yylineno, "Octal character value `%s' too big", s);
  }

  return ret;
}

_CORBA_UShort hexToWChar(char* s) {
  unsigned long ret = strtoul(s+2, 0, 16);

  return ret;
}

_CORBA_UShort escapeToWChar(char* s) {
  switch (s[1]) {
  case 'n':  return '\n';
  case 't':  return '\t';
  case 'v':  return '\v';
  case 'b':  return '\b';
  case 'r':  return '\r';
  case 'f':  return '\f';
  case 'a':  return '\a';
  case '\\': return '\\';
  case '?':  return '?';
  case '\'': return '\'';
  case '\"': return '\"';
  }
  IdlWarning(currentFile, yylineno,
	     "Behaviour for escape sequence `%s' is undefined by IDL; "
	     "using `%c'.", s, s[1]);
  return s[1];
}

char* escapedStringToString(char* s) {
  int   len = strlen(s);
  char* ret = new char[len+1];
  char  tmp[8];

  int from, to, i;

  for (from=0, to=0; from < len; from++, to++) {

    if (s[from] == '\\') {
      tmp[0] = s[from++];

      if ('0' <= s[from] && s[from] <= '7') {
	// Octal
	for (i=1;
	     i < 4 && from < len && '0' <= s[from] && s[from] <= '7';
	     from++, i++) tmp[i] = s[from];

	tmp[i]  = '\0';
	ret[to] = octalToChar(tmp);
	from--;
      }
      else if (s[from] == 'x') {
	// Hex
	tmp[1] = s[from++];
	for (i=2; i < 4 && from < len && isxdigit(s[from]); from++, i++)
	  tmp[i] = s[from];

	tmp[i]  = '\0';
	ret[to] = hexToChar(tmp);
	from--;
      }
      else if (s[from] == 'u') {
	// Wide hex not allowed
	IdlError(currentFile, yylineno,
		 "\\u may only be used in wide characters and strings");
	ret[to] = '!';
      }
      else {
	tmp[1]  = s[from];
	tmp[2]  = '\0';
	ret[to] = escapeToChar(tmp);
      }
      if (ret[to] == '\0') {
	IdlError(currentFile, yylineno, "String cannot contain \\0");
	ret[to] = '!';
      }
    }
    else ret[to] = s[from];
  }
  ret[to] = '\0';
  return ret;
}

_CORBA_UShort* escapedStringToWString(char* s) {
  int            len = strlen(s);
  _CORBA_UShort* ret = new _CORBA_UShort[len+1];
  char           tmp[8];

  int from, to, i;

  for (from=0, to=0; from < len; from++, to++) {

    if (s[from] == '\\') {
      tmp[0] = s[from++];

      if ('0' <= s[from] && s[from] <= '7') {
	// Octal
	for (i=1;
	     i < 4 && from < len && '0' <= s[from] && s[from] <= '7';
	     from++, i++) tmp[i] = s[from];

	tmp[i]  = '\0';
	ret[to] = octalToWChar(tmp);
	from--;
      }
      else if (s[from] == 'x') {
	// Hex
	tmp[1] = s[from++];
	for (i=2; i < 4 && from < len && isxdigit(s[from]); from++, i++)
	  tmp[i] = s[from];

	tmp[i]  = '\0';
	ret[to] = hexToWChar(tmp);
	from--;
      }
      else if (s[from] == 'u') {
	// Wide hex
	tmp[1] = s[from++];
	for (i=2; i < 6 && from < len && isxdigit(s[from]); from++, i++)
	  tmp[i] = s[from];

	tmp[i]  = '\0';
	ret[to] = hexToWChar(tmp);
	from--;
      }
      else {
	tmp[1]  = s[from];
	tmp[2]  = '\0';
	ret[to] = escapeToWChar(tmp);
      }
      if (ret[to] == 0) {
	IdlError(currentFile, yylineno,
		 "Wide string cannot contain wide character zero");
	ret[to] = '!';
      }
    }
    else
      ret[to] = s[from];
  }
  ret[to] = 0;
  return ret;
}

void parseLineDirective(char* s) {
  char* file    = new char[strlen(s) + 1];
  long int line = 0, mode = 0;
  int cnt       = sscanf(s, "# %ld \"%[^\"]\" %ld", &line, file, &mode);

  assert(cnt >= 1);

  if (cnt > 1) {
    if (cnt == 3) {
      if (mode == 1) {
	// New #included file
	++nestDepth;
	mainFile = 0;
	Prefix::newFile();
      }
      else if (mode == 2) {
	// Return from #include
	if (--nestDepth == 0) mainFile = 1;
	Prefix::endFile();
      }
    }
    delete [] currentFile;
    // cccp escapes \ characters, so use the normal string parser
    currentFile = escapedStringToString(file);
    delete [] file;
    if (mainFile)
      AST::tree()->setFile(currentFile);
  }
  yylineno = line;
}

int fixed(char* s) {
  return 42;
}
