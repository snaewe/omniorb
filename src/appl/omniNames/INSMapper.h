// -*- Mode: C++; -*-
//                          Package   : omniNames
// INISMapper.h             Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
//
//  This file is part of omniNames.
//
//  omniNames is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//

// $Id$
// $Log$
// Revision 1.2.2.4  2001/09/19 17:33:17  dpg1
// Update omniMapper and omniNames INS mapper to ORB core changes from a
// while ago.
//
// Revision 1.2.2.3  2001/04/19 09:39:05  sll
// Big checkin with the brand new internal APIs.
//
// Revision 1.2.2.2  2000/10/09 16:20:56  sll
// Ported to omniORB 4 API.
//
// Revision 1.2.2.1  2000/07/17 10:35:38  sll
// Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.
//
// Revision 1.3  2000/07/13 15:26:03  dpg1
// Merge from omni3_develop for 3.0 release.
//
// Revision 1.1.2.1  2000/05/24 17:13:07  dpg1
// omniNames now creates a forwarding agent with object key `NameService'
// if the root context doesn't already have that key.
//

#include <omniORB4/CORBA.h>

class INSMapper :
  public PortableServer::RefCountServantBase
{
public:

  INSMapper(PortableServer::POA_ptr inspoa, CORBA::Object_ptr obj)
    : obj_(obj)
  {
    PortableServer::ObjectId oid(11, 11, (CORBA::Octet*)"NameService", 0);
    inspoa->activate_object_with_id(oid, this);
  }

  ~INSMapper() {}

  CORBA::Boolean _dispatch(omniCallHandle&) {
    throw omniORB::LOCATION_FORWARD(CORBA::Object::_duplicate(obj_),0);
    return 1;
  }
  CORBA::Boolean _is_a(const char* id) {
    throw omniORB::LOCATION_FORWARD(CORBA::Object::_duplicate(obj_),0);
    return 1;
  }
private:
  CORBA::Object_var obj_;
};
