// -*- Mode: C++; -*-
//                            Package   : omniORB2
// optionalFeatures.h         Created on: 26/07/2001
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories, Cambridge
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
//	*** PROPRIETARY INTERFACE ***
//

#ifndef __OPTIONALFEATURES_H__
#define __OPTIONALFEATURES_H__

// Include this file to ensure that optional features in separate
// libraries are definitely linked in.
//
// The current optional features are:
//
//   OMNI_CODESETS      -- additional code set converters in the
//                         omniCodeSets library
//
//   OMNI_SSL_TRANSPORT -- SSL transport in the omnisslTP library
//
// By default, all available options are enabled by including this
// file. To turn them off, #define the relevant name to 0, e.g.:
//
//   #define OMNI_CODESETS 0


#include <omniORB4/linkHacks.h>

#ifndef OMNI_CODESETS
#define OMNI_CODESETS 1
#endif

#if(OMNI_CODESETS)
OMNI_FORCE_LINK(omniCodeSets);
#endif


#ifndef OMNI_SSL_TRANSPORT
#  ifdef OMNI_HAS_SSL_SUPPORT
#    define OMNI_SSL_TRANSPORT 1
#  else
#    define OMNI_SSL_TRANSPORT 0
#  endif
#endif

#if(OMNI_SSL_TRANSPORT)
OMNI_FORCE_LINK(omnisslTP);
#endif



#endif // __OPTIONALFEATURES_H__
