// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopMonitor.h              Created on: 23 July 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
  Revision 1.1.2.2  2002/08/21 06:23:15  dgrisby
  Properly clean up bidir connections and ropes. Other small tweaks.

  Revision 1.1.2.1  2001/07/31 16:28:02  sll
  Added GIOP BiDir support.

*/

#ifndef __GIOPMONITOR_H__
#define __GIOPMONITOR_H__

OMNI_NAMESPACE_BEGIN(omni)

class giopMonitor : public omniTask, public giopServer::Link {
public:
  giopMonitor(giopActiveCollection* c, giopServer* s) : 
    omniTask(omniTask::ImmediateDispatch),
    pd_collection(c),
    pd_server(s) { }

  void execute();

  void deactivate() { pd_collection->deactivate(); }

  giopActiveCollection* collection() const { return pd_collection; }

  static void notifyReadable(void*,giopConnection*);

private:
  giopActiveCollection*  pd_collection;
  giopServer*            pd_server;

  giopMonitor();
  giopMonitor(const giopMonitor&);
  giopMonitor& operator=(const giopMonitor&);
};

OMNI_NAMESPACE_END(omni)

#endif // __GIOPRENDEZVOUSER_H__
