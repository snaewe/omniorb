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
// Revision 1.4  2001/02/21 14:12:20  dpg1
// Merge from omni3_develop for 3.0.3 release.
//
// Revision 1.1.2.1  2000/05/24 17:13:07  dpg1
// omniNames now creates a forwarding agent with object key `NameService'
// if the root context doesn't already have that key.
//

#include <omniORB3/CORBA.h>

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

  CORBA::Boolean _dispatch(GIOP_S& giop_s) {
    throw omniORB::LOCATION_FORWARD(CORBA::Object::_duplicate(obj_));
    return 1;
  }
  CORBA::Boolean _is_a(const char* id) {
    throw omniORB::LOCATION_FORWARD(CORBA::Object::_duplicate(obj_));
    return 1;
  }
private:
  CORBA::Object_var obj_;
};
