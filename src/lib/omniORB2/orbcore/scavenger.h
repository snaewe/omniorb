// -*- Mode: C++; -*-
//                            Package   : omniORB
// scavenger.h                Created on: 5/8/97
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
// Description:
//

/*
  $Log$
  Revision 1.5.2.1  2000/07/17 10:35:59  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.6  2000/07/13 15:25:55  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.3.6.2  1999/09/24 15:01:37  djr
  Added module initialisers, and sll's new scavenger implementation.

  Revision 1.3.2.1  1999/09/21 20:37:17  sll
  -Simplified the scavenger code and the mechanism in which connections
   are shutdown. Now only one scavenger thread scans both incoming
   and outgoing connections. A separate thread do the actual shutdown.
  -omniORB::scanGranularity() now takes only one argument as there is
   only one scan period parameter instead of 2.
  -Trace messages in various modules have been updated to use the logger
   class.
  -ORBscanGranularity replaces -ORBscanOutgoingPeriod and
                                 -ORBscanIncomingPeriod.

  Revision 1.3  1999/08/30 16:47:46  sll
  New member functions.

  Revision 1.2  1999/03/11 16:25:56  djr
  Updated copyright notice

  Revision 1.1  1997/12/09 18:43:14  sll
  Initial revision

  */


#ifndef __SCAVENGER_H__
#define __SCAVENGER_H__

class ropeFactoryList;

class StrandScavenger {
public:

  static void addRopeFactories(ropeFactoryList* l);
  static void removeRopeFactories(ropeFactoryList* l);

  static int clientCallTimeLimit();
  // This number determines how long the ORB is prepare to wait before
  // giving up on a remove call on the client side and throws a
  // COMM_FAILURE exception.
  // The period starts from the moment the client thread starts marshalling
  // the call argments to the time all the result values have been
  // unmarshalled.

  static int serverCallTimeLimit();  // in no. of scan period
  // This number determines how long the ORB is prepare to wait before
  // giving up on procssing a remote call on the server side. When this
  // time limit has exceeded, the connection is simply terminated and the
  // remote end sees an adnormal connection termination and should raise
  // a COMM_FAILURE exception.
  //
  // The period starts from the moment the server thread starts unmarshalling
  // the call argments to the time all the result values have been
  // marshalled.
  //
  // The value of this variable is used to initialise the per-strand
  // variable at the beginning of processing the call. The per-strand variable
  // is scanned and decremented per scan-period by the inScavenger. When the

  static int outIdleTimeLimit();

  static int inIdleTimeLimit();

  // Concurrency contol:
  //   None. All of these functions should be considered *non-thread safe*.
  //   The behaviour of concurrent calls to the same or different functions 
  //   is undefined.
};


#endif // __SCAVENGER_H__
