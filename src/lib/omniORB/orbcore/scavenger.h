// -*- Mode: C++; -*-
//                            Package   : omniORB2
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
  static CORBA::ULong inScavengerScanPeriod();
  static void inScavengerScanPeriod(CORBA::ULong sec);

  static void initOutScavenger();
  static void pokeOutScavenger();
  static void killOutScavenger();
  static CORBA::ULong outScavengerScanPeriod();
  static void outScavengerScanPeriod(CORBA::ULong sec);
  // Concurrency contol:
  //   None. All of these functions should be considered *non-thread safe*.
  //   The behaviour of concurrent calls to the same or different functions 
  //   is undefined.
};


#endif // __SCAVENGER_H__
