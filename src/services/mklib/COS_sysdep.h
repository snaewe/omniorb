// -*- Mode: C++; -*-
//                            Package   : omniORB2
// COS_sysdep.h               Created on: 09/08/2000
//                            Author    : Sai Lai Lo (sll)
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
//      This file is included by COS stub headers.
// 	Define a set of flags in C++ macros to facilitate the use of
//      the COS stub library.

// NOTE: No inclusion guards. This file must be re-included by every
// COS header.

#if !defined(_COS_LIBRARY)

#  ifndef USE_core_stub_in_nt_dll
#    define USE_core_stub_in_nt_dll
#  endif
#  ifndef USE_dyn_stub_in_nt_dll
#    define USE_dyn_stub_in_nt_dll
#  endif
#endif

// COS stubs used to incorrectly use an external guard to only include
// this file once. We undefine the guard name here, to make sure old
// stubs work.

#undef __COS_SYSDEP_H_EXTERNAL_GUARD__
