// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopStrandFlags.h          Created on: 2006-06-16
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2006 Apasphere Ltd.
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
//    Flags values used in the giopStrand flags field. For use by
//    interceptors to maintain per-connection information.
//

#ifndef __GIOPSTRANDFLAGS_H__
#define __GIOPSTRANDFLAGS_H__


//
// Application specific space
//

#define GIOPSTRAND_APPLICATION_MASK 0xff000000
// The top 8 bits of the flags are reserved for application-specific
// use. Flags below will never be allocated in this space.


//
// Allocated flag bits
//

#define GIOPSTRAND_ENABLE_TRANSPORT_BATCHING (1 << 0)
// Normally omniORB sets connections to send data as soon as possible,
// e.g. with the TCP_NODELAY socket option. This flag means that
// message batching in the transport should be enabled.

#define GIOPSTRAND_HOLD_OPEN (1 << 1)
// If set, the connection is held open, rather than being scavenged
// when idle.



#endif // __GIOPSTRANDFLAGS_H__
