// -*- Mode: C++; -*-
//                            Package   : omniORB
// dynamicLib.h               Created on: 15/9/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//    02111-1307, USA
//
//
// Description:
//    Hook to allow orbcore to access dynamic library when linked.
//

/*
  $Log$
  Revision 1.3  2000/07/13 15:26:03  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.2  2000/02/08 13:25:24  djr
  Added a couple of missing _core_attr.

  Revision 1.1.2.1  1999/09/22 14:26:25  djr
  Major rewrite of orbcore to support POA.

*/

#ifndef __DYNAMICLIB_H__
#define __DYNAMICLIB_H__


#if defined(_OMNIORB_LIBRARY)
# define _core_attr
#elif defined(_OMNIORB_DYNAMIC_LIBRARY)
# define _core_attr _OMNIORB_NTDLL_IMPORT
#else
# error "Neither _OMNIORB_LIBRARY nor _OMNIORB_DYNAMIC_LIBRARY defined"
#endif


class omniCallDescriptor;


class omniDynamicLib {
public:

  // This points to the implementation of dynamic library functions
  // that the orbcore library should use.  If the dynamic library
  // is linked then the real functions are called.  Otherwise
  // stub versions are called which do nothing, or deal with the
  // error appropriately.
  static _core_attr omniDynamicLib* ops;

  // This points to the dynamic library's version of the functions
  // if the dynamic is linked, or is zero otherwise.
  static _core_attr omniDynamicLib* hook;

  // This is used in the dynamic library only to ensure that if
  // the dynamic library is linked, then the hook will be enagaged.
  // (see ** below).
  //  NB. Since it is _only_ used in the dynamic library, we don't
  // need a _dyn_attr here.
  static char link_in;

  ////////////////////////////////////////////////
  // Operations exported by the dynamic library //
  ////////////////////////////////////////////////

  // Initialisation
  void (*init)();
  void (*deinit)();

  // Marshalling contexts.
  size_t (*context_aligned_size)(size_t initoffset, CORBA::Context_ptr cxtx,
				 const char*const* which, int how_many);
  void (*marshal_context)(NetBufferedStream&, CORBA::Context_ptr cxtx,
			  const char*const* which, int how_many);

  // Local call call-back function for CORBA::Repository::lookup_id().
  // This is needed to support the server side of _get_interface(),
  // in the case that the interface repository is co-located!
  void (*lookup_id_lcfn)(omniCallDescriptor*, omniServant*);
};


#ifdef _OMNIORB3_DYNAMIC_LIBRARY
// ** Any source file in the dynamic library that includes
// this header is guarenteed to engage the dynamic library
// hook.  Thus this header should be included in source files
// which include code which is used by the functions exported
// under this interface.
static char* omniDynamicLib_link_in = &omniDynamicLib::link_in;
#endif


#undef _core_attr


#endif  // __DYNAMICLIB_H__
