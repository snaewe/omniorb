// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyCodeSets.cc              Created on: 2002/09/06
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2002 Apasphere Ltd.
//
//    This file is part of the omniORBpy library
//
//    The omniORBpy library is free software; you can redistribute it
//    and/or modify it under the terms of the GNU Lesser General
//    Public License as published by the Free Software Foundation;
//    either version 2.1 of the License, or (at your option) any later
//    version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
//    MA 02111-1307, USA
//
// Description:
//    Codesets library

// $Id$

// $Log$
// Revision 1.1.2.1  2002/09/06 21:34:26  dgrisby
// Add codesets and sslTP modules.
//

#ifdef __WIN32__
#define DLL_EXPORT _declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#if defined(__VMS)
#include <Python.h>
#else
#include PYTHON_INCLUDE
#endif

#include <omniORB4/linkHacks.h>

OMNI_FORCE_LINK(omniCodeSets);


extern "C" {
  static PyMethodDef omnicodesets_methods[] = {
    {0,0}
  };


  void DLL_EXPORT init_omnicodesets()
  {
    PyObject* m = Py_InitModule((char*)"_omnicodesets", omnicodesets_methods);
  }
};
