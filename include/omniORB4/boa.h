// -*- Mode: C++; -*-
//                            Package   : omniORB2
// boa.h                      Created on: 27/7/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//

/*
  $Log$
  Revision 1.2.2.2.2.1  2001/02/23 16:50:44  sll
  SLL work in progress.

  Revision 1.2.2.2  2000/11/09 12:27:49  dpg1
  Huge merge from omni3_develop, plus full long long from omni3_1_develop.

  Revision 1.2.2.1  2000/07/17 10:35:34  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:26:05  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.2  2000/03/03 14:29:16  djr
  Improvement to BOA skeletons (less generated code).

  Revision 1.1.2.1  1999/09/24 09:51:42  djr
  Moved from omniORB2 + some new files.

*/

#ifndef __OMNIORB_BOA_H__
#define __OMNIORB_BOA_H__

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
////////////////////////// omniOrbBoaServant /////////////////////////
//////////////////////////////////////////////////////////////////////

class omniOrbBoaServant : public virtual omniServant {
public:
  virtual ~omniOrbBoaServant();

  omniOrbBoaServant();
  inline omniOrbBoaServant(const omniOrbBoaKey& k)
    : pd_key(k) {}

  void _dispose();


  //////////////////////
  // omniORB Internal //
  //////////////////////

  void _obj_is_ready();
  // Implementation of _obj_is_ready(BOA_ptr);

  void* _this(const char* repoId);
  // If activated, return a <foo_ptr> (where foo is given in the
  // <repoId>).  Otherwise throws BAD_INV_ORDER.

  inline const omniOrbBoaKey& _key() const { return pd_key; }
  inline void _obj_is_ready(CORBA::BOA_ptr) { _obj_is_ready(); }
  inline CORBA::BOA_ptr _boa() { return CORBA::BOA::getBOA(); }

private:
  virtual omniObjRef* _do_get_interface();
  // Overrides omniServant.

protected:
  omniOrbBoaKey pd_key;
  // This is used only for construction/initialisation.
  // When the object is activated, the key in the id is
  // what counts.
  //  Immutable.
};

OMNI_NAMESPACE_END(omni)

#endif  // __OMNIORB_BOA_H__
