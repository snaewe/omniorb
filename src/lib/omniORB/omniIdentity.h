// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniIdentity.h             Created on: 22/2/99
//                            Author    : David Riddoch (djr)
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
//    Base class for the identity of an object implementation (which
//    may be local or remote).
//

/*
  $Log$
  Revision 1.2.2.3  2000/11/03 19:08:52  sll
  Use virtual dtor with gcc.

  Revision 1.2.2.2  2000/09/27 17:39:28  sll
  Updated include/omniORB3 to include/omniORB4

  Revision 1.2.2.1  2000/07/17 10:35:40  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:26:03  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.1  1999/09/22 14:26:26  djr
  Major rewrite of orbcore to support POA.

*/

#ifndef __OMNIIDENTITY_H__
#define __OMNIIDENTITY_H__


class omniCallDescriptor;
class omniObjRef;


class omniIdentity {
public:
  inline const _CORBA_Octet* key() const  { return pd_key.key();    }
  inline int keysize() const              { return pd_key.size();   }
  //  These functions are thread-safe.

  inline int is_equal(const _CORBA_Octet* key, int keysize) const {
    return pd_key.is_equal(key, keysize);
  }

  virtual void dispatch(omniCallDescriptor&) = 0;
  // Dispatch a call from an object reference.  Grabs a reference
  // to this identity, which it holds until the call returns.
  //  Must hold <omni::internalLock> on entry.  It is not held
  // on exit.

  virtual void gainObjRef(omniObjRef*) = 0;
  virtual void loseObjRef(omniObjRef*) = 0;
  // Must hold <omni::internalLock>.


protected:
#ifndef __GNUG__ 
  inline ~omniIdentity() {}
  // Should only be destroyed by implementation of derived classes.
  // This doesn't need to be virtual, since it is only ever deleted
  // by the most derived type.
#else
  virtual ~omniIdentity() {}
  // But gcc is rather anal and insits on a class with virtual functions must
  // have a virtual dtor.
#endif

  inline omniIdentity(omniObjKey& key)
    : pd_key(key, 1) {}
  // May consume <key>.

  inline omniIdentity(const _CORBA_Octet* key, int keysize)
    : pd_key(key, keysize) {}
  // Copies <key>.

  inline omniIdentity(_CORBA_Octet* key, int keysize)
    : pd_key(key, keysize) {}
  // Consumes <key>.

private:
  omniIdentity(const omniIdentity&);
  omniIdentity& operator = (const omniIdentity&);
  // Not implemented.


  omniObjKey pd_key;
  // Immutable.
};


#endif  // __OMNIIDENTITY_H__
