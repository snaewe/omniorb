// -*- Mode: C++; -*-
//                            Package   : omniORB
// excepthandler.h            Created on: 13/5/97
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
  Revision 1.1.4.1  2001/04/18 17:18:17  sll
  Big checkin with the brand new internal APIs.
  These files were relocated and scoped with the omni namespace.

  Revision 1.3.2.1  2000/07/17 10:35:53  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.4  2000/07/13 15:25:58  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.2.8.1  1999/09/22 14:26:48  djr
  Major rewrite of orbcore to support POA.

  Revision 1.1  1997/12/09 18:43:07  sll
  Initial revision

  */

#ifndef __EXCEPTHANDLER_H__
#define __EXCEPTHANDLER_H__



class omniExHandlers_iterator;


class omniExHandlers {
public:
  void*                   transient_hdr;
  void*                   transient_cookie;
  void*                   commfail_hdr;
  void*                   commfail_cookie;
  void*                   sysexcpt_hdr;
  void*                   sysexcpt_cookie;
  static omniExHandlers** Table;
  static omni_mutex       TableLock;
  friend class omniExHandlers_iterator;
private:
  omniExHandlers();
  omniObjRef*             objptr;
  omniExHandlers*         next;
};


class omniExHandlers_iterator {
public:
  omniExHandlers_iterator();
  // Concurrency control:
  //       MUTEX = omniExHandlers::TableLock
  // Pre-condition:
  //       Does not hold <MUTEX> on entry
  // Post-condition:
  //       Hold <MUTEX> on exit

  ~omniExHandlers_iterator();
  // Concurrency control:
  //       MUTEX = omniExHandlers::TableLock
  // Pre-condition:
  //       Hold <MUTEX> on entry
  // Post-condition:
  //       Does not hold <MUTEX> on exit

  omniExHandlers* find_or_create(omniObjRef* p);
  // Locate the omniExHandlers for this object
  // If found just returns
  // else create a new record, initialise all the handler fields to null and
  // returns the new record.

  omniExHandlers* find(omniObjRef* p);
  // Locate the omniExHandlers for this object
  // Return 0 if not found.

  void remove(omniObjRef* p);
  // Locate the omniExHandlers for this object, if found, delete the record

};


#endif // __EXCEPTHANDLER_H__
