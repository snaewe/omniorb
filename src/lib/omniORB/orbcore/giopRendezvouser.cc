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
  Revision 1.1.2.1  2001/02/23 16:46:58  sll
  Added new files.

*/

#include <omniORB4/CORBA.h>
#include <invoker.h>
#include <giopServer.h>
#include <giopRendezvouser.h>

OMNI_NAMESPACE_BEGIN(omni)

void
giopRendezvouser::execute() {
  CORBA::Boolean exit_on_error;

  do {
    exit_on_error = 0;
    giopConnection* newconn = 0;
    try {
      newconn = pd_endpoint->accept();
      if (newconn)
	pd_server->notifyRzNewConnection(this,newconn);
      else {
	exit_on_error = 1;
	break;
      }
    }
    catch(const giopServer::outOfResource&) {
      delete newconn;
    }
    catch(const giopServer::Terminate&) {
      delete newconn;
      break;
    }
    catch(...) {
      // Catch all unexpected error conditions. Reach here means that we
      // should not continue!
      if( omniORB::trace(0) ) {
	omniORB::logger l;
	l << "Unexpected exception caught by giopRendezvouser\n";
      }
      if (newconn) delete newconn;
      exit_on_error = 1;
      break;
    }
  } while(!pd_singleshot);

  pd_server->notifyRzDone(this,exit_on_error);
}

void
giopRendezvouser::terminate() {
  pd_endpoint->poke();
}


OMNI_NAMESPACE_END(omni)
