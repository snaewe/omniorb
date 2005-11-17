// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopRendezvouser.cc        Created on: 20 Dec 2000
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
  Revision 1.1.6.2  2005/11/17 17:03:26  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.6.1  2003/03/23 21:02:15  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.4.6  2002/09/09 22:11:50  dgrisby
  SSL transport cleanup even if certificates are wrong.

  Revision 1.1.4.5  2002/08/21 06:23:15  dgrisby
  Properly clean up bidir connections and ropes. Other small tweaks.

  Revision 1.1.4.4  2002/03/13 16:05:39  dpg1
  Transport shutdown fixes. Reference count SocketCollections to avoid
  connections using them after they are deleted. Properly close
  connections when in thread pool mode.

  Revision 1.1.4.3  2001/07/13 15:26:18  sll
  notifyReadable now really tells the server a connection is ready to be
  read. Use AcceptAndMonitor instead of Accept.

  Revision 1.1.4.2  2001/06/20 18:35:18  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.4.1  2001/04/18 18:10:50  sll
  Big checkin with the brand new internal APIs.

*/

#include <omniORB4/CORBA.h>
#include <invoker.h>
#include <giopServer.h>
#include <giopRendezvouser.h>

OMNI_NAMESPACE_BEGIN(omni)

void
giopRendezvouser::notifyReadable(void* this_,giopConnection* conn) {

  giopRendezvouser* r = (giopRendezvouser*)this_;
  r->pd_server->notifyRzReadable(conn);
}

void
giopRendezvouser::execute()
{
  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "giopRendezvouser task execute for "
      << pd_endpoint->address() << "\n";
  }

  CORBA::Boolean exit_on_error;

  do {
    exit_on_error = 0;
    giopConnection* newconn = 0;
    try {
      newconn = pd_endpoint->AcceptAndMonitor(notifyReadable,this);
      if (newconn) {
	pd_server->notifyRzNewConnection(this,newconn);
      }
      else {
	exit_on_error = 1;
	break;
      }
    }
    catch(const giopServer::outOfResource&) {
      // giopServer has consumed the connection.
    }
    catch(const giopServer::Terminate&) {
      newconn->decrRefCount(1);
      break;
    }
    catch(...) {
      // Catch all unexpected error conditions. Reach here means that we
      // should not continue!
      if( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "Unexpected exception caught by giopRendezvouser\n";
      }
      if (newconn) {
	newconn->decrRefCount(1);
      }
      exit_on_error = 1;
      break;
    }
  } while(!pd_singleshot);

  pd_server->notifyRzDone(this,exit_on_error);
}

void
giopRendezvouser::terminate() {
  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "giopRendezvouser for " << pd_endpoint->address()
      << " terminate...\n";
  }
  pd_endpoint->Poke();
}


OMNI_NAMESPACE_END(omni)
