// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopMonitor.cc             Created on: 21 Dec 2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
//	*** PROPRIETORY INTERFACE ***
// 

/*
  $Log$
  Revision 1.1.2.1  2001/07/31 16:36:01  sll
  Added GIOP BiDir support.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <invoker.h>
#include <giopServer.h>
#include <giopMonitor.h>

OMNI_NAMESPACE_BEGIN(omni)

void
giopMonitor::notifyReadable(void* this_,giopConnection* conn) {
  giopMonitor* m = (giopMonitor*)this_;
  m->pd_server->notifyRzReadable(conn);
}

void
giopMonitor::execute() {
  CORBA::Boolean exit_on_error = 0;

  try {
    pd_collection->Monitor(notifyReadable,this);
  }
  catch(...) {
    // Catch all unexpected error conditions. Reach here means that we
    // should not continue!
    if( omniORB::trace(0) ) {
      omniORB::logger l;
      l << "Unexpected exception caught by giopMonitor\n";
    }
    exit_on_error = 1;
  };

  pd_server->notifyMrDone(this,exit_on_error);
}


OMNI_NAMESPACE_END(omni)