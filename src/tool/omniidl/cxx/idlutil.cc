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

_CORBA_WChar* idl_wstrdup(const _CORBA_WChar* s)
{
  if (s) {
    int i, len;
    for (len=0; s[len]; len++);
    _CORBA_WChar* ret = new _CORBA_WChar[len+1];
    for (i=0; i<len; i++)
      ret[i] = s[i];
    ret[i] = 0;
    return ret;
  }
  else
    return 0;
}

int idl_wstrlen(const _CORBA_WChar* s)
{
  int l;
  for (l=0; *s; ++s, ++l);
  return l;
}

_CORBA_WChar* idl_wstrcpy(_CORBA_WChar* a, const _CORBA_WChar* b)
{
  _CORBA_WChar* r = a;
  for (; *b; ++a, ++b) *a = *b;
  *a = 0;
  return r;
}


_CORBA_WChar* idl_wstrcat(_CORBA_WChar* a, const _CORBA_WChar* b)
{
  _CORBA_WChar* r = a;
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
