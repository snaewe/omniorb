// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopRendezvouser.h           Created on: 20 Dec 2000
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
  Revision 1.1.4.1  2001/04/18 17:19:00  sll
  Big checkin with the brand new internal APIs.

  Revision 1.1.2.1  2001/02/23 16:47:05  sll
  Added new files.

*/

#ifndef __GIOPRENDEZVOUSER_H__
#define __GIOPRENDEZVOUSER_H__

OMNI_NAMESPACE_BEGIN(omni)

class giopRendezvouser : public omniTask, public giopServer::Link {
public:
  giopRendezvouser(giopEndpoint* e, giopServer* s, CORBA::Boolean h=0) : 
    omniTask(omniTask::ImmediateDispatch),
    pd_endpoint(e),
    pd_server(s),
    pd_singleshot(h) { }

  void execute();

  void terminate();
  // finish off this task, call by another thread

  giopEndpoint* endpoint() const { return pd_endpoint; }

private:
  giopEndpoint*          pd_endpoint;
  giopServer*            pd_server;
  const CORBA::Boolean   pd_singleshot;

  giopRendezvouser();
  giopRendezvouser(const giopRendezvouser&);
  giopRendezvouser& operator=(const giopRendezvouser&);
};

OMNI_NAMESPACE_END(omni)

#endif // __GIOPRENDEZVOUSER_H__
