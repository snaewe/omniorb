// -*- Mode: C++; -*-
//                            Package   : omniORB2
// reDirect.h                 Created on: 19/3/97
//                            Author    : Sai Lai Lo (sll)
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
  Revision 1.1  1997/09/20 17:44:37  dpg1
  Initial revision

  */

#ifndef __REDIRECT_H__
#define __REDIRECT_H__

#include <omniORB2/CORBA.h>

class reDirect : public virtual omniObject, public virtual CORBA::Object {
public:

  reDirect(CORBA::Object_ptr fwdref); 

  reDirect(CORBA::Object_ptr fwdref,const omniORB::objectKey& mykey);

  virtual ~reDirect() { }

  CORBA::Object_ptr forwardReference() const;

  CORBA::Object_ptr _this();
  void _obj_is_ready(CORBA::BOA_ptr boa);
  CORBA::BOA_ptr _boa();
  void _dispose();
  omniORB::objectKey _key();
  virtual CORBA::Boolean dispatch(GIOP_S &s, const char *,CORBA::Boolean);

private:
  CORBA::Object_var pd_fwdref;
  reDirect();
};

#endif // __REDIRECT_H__
