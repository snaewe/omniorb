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
  Revision 1.3.6.1  1999/09/22 14:27:08  djr
  Major rewrite of orbcore to support POA.

  Revision 1.3  1999/08/30 16:47:46  sll
  New member functions.

  Revision 1.2  1999/03/11 16:25:56  djr
  Updated copyright notice

  Revision 1.1  1997/12/09 18:43:14  sll
  Initial revision

  */


#ifndef __SCAVENGER_H__
#define __SCAVENGER_H__

class StrandScavenger {
public:
  static void initInScavenger();
  static void pokeInScavenger();
  static void killInScavenger();

  static void initOutScavenger();
  static void pokeOutScavenger();
  static void killOutScavenger();

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
