// -*- Mode: C++; -*-
//                            Package   : omniORB
// context.h                  Created on: 9/1998
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
 Revision 1.5.2.2  2000/09/27 17:25:40  sll
 Changed include/omniORB3 to include/omniORB4.

 Revision 1.5.2.1  2000/07/17 10:35:40  sll
 Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

 Revision 1.6  2000/07/13 15:26:02  dpg1
 Merge from omni3_develop for 3.0 release.

 Revision 1.4.6.1  1999/09/22 14:26:29  djr
 Major rewrite of orbcore to support POA.

 Revision 1.4  1999/05/25 17:48:50  sll
 Now use _CORBA_PseudoValue_Sequence.

 Revision 1.3  1999/04/21 11:22:14  djr
 CORBA::Context is a friend of ContextImpl

*/

#ifndef __CONTEXT_H__
#define __CONTEXT_H__


class ContextImpl : public CORBA::Context {
public:
  ContextImpl(const char* name, CORBA::Context_ptr parent);
  virtual ~ContextImpl();

  virtual const char* context_name() const;
  virtual CORBA::Context_ptr parent() const;
  virtual CORBA::Status create_child(const char*, CORBA::Context_out);
  virtual CORBA::Status set_one_value(const char*, const CORBA::Any&);
  virtual CORBA::Status set_values(CORBA::NVList_ptr);
  virtual CORBA::Status delete_values(const char*);
  virtual CORBA::Status get_values(const char* start_scope,
				   CORBA::Flags op_flags,
				   const char* pattern,
				   CORBA::NVList_out values);
  virtual CORBA::Boolean NP_is_nil() const;
  virtual CORBA::Context_ptr NP_duplicate();

  ///////////////////
  // omni internal //
  ///////////////////
  const char* lookup_single(const char* name) const;
  // Lookup single entry, starting at current scope, and looking in
  // parent/grandparents etc. if necassary.
  //  Retains ownership of the return value. Returns 0 if not found.

  void insert_single_consume(char* name, char* value);
  // Inserts a single entry, consuming the given name/value pair.

  void decrRefCount();
  // Decrease the reference count, and delete this context
  // if there are no more references or children.
  //  Must not hold <pd_lock>.

private:
  friend class CORBA::Context;

  ContextImpl(const ContextImpl&);             // not implemented
  ContextImpl& operator=(const ContextImpl&);  // not implemented

  int matchPattern(const char* pat, CORBA::ULong& bottom,
		   CORBA::ULong& top) const;
  // Returns 1 if finds a match, or 0 if no match.
  // The index of the first match is in <bottom>, and the last
  // match + 1 in <top>.
  //  Must hold <pd_lock>.

  static void add_values(ContextImpl* c, CORBA::Flags op_flags,
			 const char* pattern, int wildcard,
			 CORBA::NVList_ptr val_list);
  // If( wildcard ), then pattern has terminating '*', and all matching
  // name-value pairs are added to val_list. Recurses on parent of context
  // if allowed by op_flags.
  //  If( !wildcard ), looks for a single match starting at the given
  // context, looking in parent if it is not found and op_flags allows.

  void check_context_name(const char* n);
  void check_property_name(const char* n);
  // These check that the given (non-null) identifier is a valid context
  // name or property name respectively.

  void addChild(ContextImpl* c) {
    omni_mutex_lock lock(pd_lock);
    c->pd_nextSibling = pd_children;
    pd_children = c;
  }
  // Add the given ContextImpl into the list of children.

  void loseChild(ContextImpl* child);
  // The given child is dying - remove from the list of dependents. This
  // must only be called by a child of this context from its d'tor.

  struct Entry {
    char* name;
    char* value;
  };
  typedef _CORBA_PseudoValue_Sequence<Entry> EntrySeq;

  CORBA::String_var  pd_name;         // set once - never changes
  CORBA::Context_ptr pd_parent;       // set once - never changes
  EntrySeq           pd_entries;      // sorted list of entries
  ContextImpl*       pd_children;     // list of Ctxts which depend on this
  ContextImpl*       pd_nextSibling;  // linked list of siblings
  unsigned           pd_refCount;

  omni_mutex         pd_lock;

  // Manages access to <pd_entries>, <pd_children>, <pd_refCount> and the
  // <pd_nextSibling> pointers in its children.
};


#endif  // __CONTEXT_H__
