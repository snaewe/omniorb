// -*- c++ -*-
//                          Package   : omniidl
// idlutil.cc               Created on: 1999/10/11
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
//   Utility functions

// $Id$
// $Log$
// Revision 1.3.2.4  2001/02/20 17:39:57  dpg1
// FreeBSD update -- use strtouq instead of strtoull.
//
// Revision 1.3.2.3  2000/10/24 09:53:31  dpg1
// Clean up omniidl system dependencies. Replace use of _CORBA_ types
// with IDL_ types.
//
// Revision 1.3.2.2  2000/09/22 10:50:21  dpg1
// Digital Unix uses strtoul, not strtoull
//
// Revision 1.3.2.1  2000/08/07 15:34:36  dpg1
// Partial back-port of long long from omni3_1_develop.
//
// Revision 1.3  1999/11/04 17:16:54  dpg1
// Changes for NT.
//
// Revision 1.2  1999/11/02 17:07:24  dpg1
// Changes to compile on Solaris.
//
// Revision 1.1  1999/10/27 14:05:54  dpg1
// *** empty log message ***
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <idlutil.h>

char* idl_strdup(const char* s)
{
  if (s) {
    char* ret = new char[strlen(s)+1];
    strcpy(ret, s);
    return ret;
  }
  else
    return 0;
}

IDL_WChar* idl_wstrdup(const IDL_WChar* s)
{
  if (s) {
    int i, len;
    for (len=0; s[len]; len++);
    IDL_WChar* ret = new IDL_WChar[len+1];
    for (i=0; i<len; i++)
      ret[i] = s[i];
    ret[i] = 0;
    return ret;
  }
  else
    return 0;
}

int idl_wstrlen(const IDL_WChar* s)
{
  int l;
  for (l=0; *s; ++s, ++l);
  return l;
}

IDL_WChar* idl_wstrcpy(IDL_WChar* a, const IDL_WChar* b)
{
  IDL_WChar* r = a;
  for (; *b; ++a, ++b) *a = *b;
  *a = 0;
  return r;
}


IDL_WChar* idl_wstrcat(IDL_WChar* a, const IDL_WChar* b)
{
  IDL_WChar* r = a;
  for (; *a; ++a);
  for (; *b; ++a, ++b) *a = *b;
  *a = 0;
  return r;
}

#ifdef NO_STRCASECMP
#include <ctype.h>

int strcasecmp(const char* s1, const char* s2)
{
  for (; *s1 && *s2; ++s1, ++s2)
    if (toupper(*s1) != toupper(*s2))
      break;

  if      (!*s1 && !*s2)                return 0;
  else if (toupper(*s1) < toupper(*s2)) return -1;
  else                                  return 1;
}
#endif


#ifdef HAS_LongLong

#  if defined(__WIN32__)

IdlIntLiteral
idl_strtoul(const char* text, int base)
{
  IdlIntLiteral ull;
  switch (base) {
  case 8:
    sscanf(text, "%I64o", &ull);
    break;
  case 10:
    sscanf(text, "%I64d", &ull);
    break;
  case 16:
    sscanf(text, "%I64x", &ull);
    break;
  default:
    abort();
  }
  return ull;
}

#  elif defined(__osf1__)

IdlIntLiteral
idl_strtoul(const char* text, int base)
{
  return strtoul(text, 0, base);
}

#  elif defined(__freebsd__)

IdlIntLiteral
idl_strtoul(const char* text, int base)
{
  return strtouq(text, 0, base);
}

#  else

IdlIntLiteral
idl_strtoul(const char* text, int base)
{
  return strtoull(text, 0, base);
}

#  endif

#else

// No long long support

IdlIntLiteral
idl_strtoul(const char* text, int base)
{
  return strtoul(text, 0, base);
}

#endif


IdlFloatLiteral
idl_strtod(const char* text)
{
  // *** Should cope with long double
  return strtod(text,0);
}
