// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniAMI.h                  Created on: 21/8/2000
//                            Author    : David Scott (djs)
//
//    Copyright (C) 1996-2000 AT&T Laboratories Cambridge
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
//    Runtime AMI support
//
/*
 $Id$
 $Log$
 Revision 1.1.2.5  2000/09/28 18:28:37  djs
 Bugfixes in Poller (wrt timout behaviour and is_ready function)
 Removed traces of Private POA/ internal ReplyHandler servant for Poller
 strategy
 General comment tidying

*/

#ifndef __omniAMI_h__
#define __omniAMI_h__

#include <omniORB3/callDescriptor.h>
#include <omniORB3/omniObjRef.h>
#include "queue.h"

class omniAMICall;
class omni_AMI_initialiser;

class AMI{

 public:
  // AMI calls are stored in a Queue...
  typedef Queue<omniAMICall*> Queue;
  
  // ... and processed by a number of system threads
  class Worker: public omni_thread{
  private:
    omniAMICall *first_task;

  public:
    ~Worker() { } // silence stupid compiler warning
    Worker(omniAMICall *cd);
    void process(omniAMICall& call);
    void shutdown();

    void *run_undetached(void *arg);
  };

  // Called by stubs to add an AMI call to the Queue
  static void enqueue(omniAMICall *cd);

  // Shuts down the worker threads and destroys the Queue
  static void shutdown();

  friend class omni_AMI_initialiser;
};


#endif /* __omniAMI_h__ */


