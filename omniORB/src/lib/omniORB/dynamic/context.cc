// -*- Mode: C++; -*-
//                            Package   : omniORB
// context.cc                 Created on: 9/1998
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2004-2006 Apasphere Ltd
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
//   Implementation of CORBA::Context.
//

/*
 $Log$
 Revision 1.14.2.3  2006/04/28 18:40:46  dgrisby
 Merge from omni4_0_develop.

 Revision 1.14.2.2  2004/10/13 17:58:20  dgrisby
 Abstract interfaces support; values support interfaces; value bug fixes.

 Revision 1.14.2.1  2003/03/23 21:02:51  dgrisby
 Start of omniORB 4.1.x development branch.

 Revision 1.12.2.13  2002/01/16 11:31:56  dpg1
 Race condition in use of registerNilCorbaObject/registerTrackedObject.
 (Reported by Teemu Torma).

 Revision 1.12.2.12  2001/11/27 14:35:07  dpg1
 Context, DII fixes.

 Revision 1.12.2.11  2001/11/06 15:41:35  dpg1
 Reimplement Context. Remove CORBA::Status. Tidying up.

 Revision 1.12.2.10  2001/09/24 10:41:08  dpg1
 Minor codes for Dynamic library and omniORBpy.

 Revision 1.12.2.9  2001/09/19 17:26:44  dpg1
 Full clean-up after orb->destroy().

 Revision 1.12.2.8  2001/08/17 17:07:05  sll
 Remove the use of omniORB::logStream.

 Revision 1.12.2.7  2001/06/13 20:10:03  sll
 Minor update to make the ORB compiles with MSVC++.

 Revision 1.12.2.6  2001/04/19 09:14:16  sll
 Scoped where appropriate with the omni namespace.

 Revision 1.12.2.5  2000/12/05 17:41:00  dpg1
 New cdrStream functions to marshal and unmarshal raw strings.

 Revision 1.12.2.4  2000/11/03 19:07:31  sll
 Use new marshalling functions for byte, octet and char. Use get_octet_array
 instead of get_char_array.

 Revision 1.12.2.3  2000/10/06 16:40:53  sll
 Changed to use cdrStream.

 Revision 1.12.2.2  2000/09/27 17:25:40  sll
 Changed include/omniORB3 to include/omniORB4.

 Revision 1.12.2.1  2000/07/17 10:35:40  sll
 Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

 Revision 1.13  2000/07/13 15:26:03  dpg1
 Merge from omni3_develop for 3.0 release.

 Revision 1.9.6.4  2000/06/22 10:40:11  dpg1
 exception.h renamed to exceptiondefs.h to avoid name clash on some
 platforms.

 Revision 1.9.6.3  1999/10/29 13:18:10  djr
 Changes to ensure mutexes are constructed when accessed.

 Revision 1.9.6.2  1999/10/14 16:21:55  djr
 Implemented logging when system exceptions are thrown.

 Revision 1.9.6.1  1999/09/22 14:26:29  djr
 Major rewrite of orbcore to support POA.

 Revision 1.9  1999/06/26 18:03:30  sll
 Corrected minor bug in marshal.

 Revision 1.8  1999/06/25 13:50:24  sll
 Renamed compatibility flag to omniORB_27_CompatibleAnyExtraction.

 Revision 1.7  1999/06/22 14:59:03  sll
 set_one_value now correctly use the any extraction operator for string.
 It takes into account of the configuration variable copyStringInAnyExtraction.

 Revision 1.6  1999/05/25 17:39:35  sll
 Added check for invalid arguments using magic number.

 Revision 1.5  1999/04/21 11:24:37  djr
 Added marshalling methods, plus a few minor mods.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/objTracker.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <context.h>
#include <pseudo.h>
#include <exceptiondefs.h>

#include <ctype.h>
#include <libcWrapper.h>
#include <orbParameters.h>

#define INIT_MAX_SEQ_LENGTH  6


OMNI_NAMESPACE_BEGIN(omni)

static ContextImpl* default_context = 0;

ContextImpl::ContextImpl(const char* name, CORBA::Context_ptr parent)
{
  if( !name )  name = "";
  else if( *name )  check_context_name(name);

  pd_name = CORBA::string_dup(name);
  pd_parent = parent;
  pd_entries.length(INIT_MAX_SEQ_LENGTH);
  pd_entries.length(0);
  pd_children = 0;
  pd_nextSibling = 0;
  pd_refCount = 1;
  if( !CORBA::is_nil(pd_parent) )
    ((ContextImpl*)pd_parent)->addChild(this);
}


ContextImpl::~ContextImpl()
{
  // This destructor can only be called when the reference count
  // has gone to zero, and there are no children.
  if( pd_refCount || pd_children )
    throw omniORB::fatalException(__FILE__, __LINE__,
		  "Application deleted a CORBA::Context explicitly");

  OMNIORB_USER_CHECK(this != default_context);
  // This fails if the application releases the default context too many times.

  // <pd_name> freed by String_var.
  // We don't own <pd_parent>.

  for( CORBA::ULong i = 0; i < pd_entries.length(); i++ ){
    if( pd_entries[i].name  )  CORBA::string_free(pd_entries[i].name);
    if( pd_entries[i].value )  CORBA::string_free(pd_entries[i].value);
  }

  if( !CORBA::is_nil(pd_parent) )
    ((ContextImpl*)pd_parent)->loseChild(this);
}

void
ContextImpl::releaseDefault()
{
  if (default_context) {
    ContextImpl* d = default_context;
    default_context = 0;
    d->decrRefCount();
    omniORB::logs(15, "Released default Context");
  }
}


const char*
ContextImpl::context_name() const
{
  // Never changes - no need to lock.
  return pd_name;
}


CORBA::Context_ptr
ContextImpl::parent() const
{
  // Never changes - no need to lock.
  return pd_parent;
}


void
ContextImpl::create_child(const char* name, CORBA::Context_out out)
{
  out = CORBA::Context::_nil();
  // The c'tor checks that the name is legal.
  out = new ContextImpl(name, this);
}


void
ContextImpl::set_one_value(const char* prop_name, const CORBA::Any& value)
{
  // Check the property name is valid.
  if( !prop_name )  OMNIORB_THROW(BAD_PARAM,
				  BAD_PARAM_NullStringUnexpected,
				  CORBA::COMPLETED_NO);
  check_property_name(prop_name);

  CORBA::String_var name(CORBA::string_dup(prop_name));

  const char* strval;
  if( !(value >>= strval) )
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_AnyDoesNotContainAString,
		  CORBA::COMPLETED_NO);

  insert_single_consume(name._retn(), CORBA::string_dup(strval));
}


void
ContextImpl::set_values(CORBA::NVList_ptr values)
{
  if( !CORBA::NVList::PR_is_valid(values) || CORBA::is_nil(values) )
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidNVList, CORBA::COMPLETED_NO);

  CORBA::ULong count = values->count();

  for( CORBA::ULong i = 0; i < count; i++ ){
    CORBA::NamedValue_ptr nv = values->item(i);
    set_one_value(nv->name(), *nv->value());
  }
}


void
ContextImpl::delete_values(const char* pattern)
{
  omni_tracedmutex_lock lock(pd_lock);
  CORBA::ULong bottom, top;

  if( !matchPattern(pattern, bottom, top) )
    OMNIORB_THROW(BAD_CONTEXT,
		  BAD_CONTEXT_NoMatchingProperty,
		  CORBA::COMPLETED_NO);

  CORBA::ULong nmatches = top - bottom;
  CORBA::ULong count = pd_entries.length();

  // delete entries, and move ones above down
  for( CORBA::ULong i = bottom; i  + nmatches < count; i++ ){
    CORBA::string_free(pd_entries[i].name);
    CORBA::string_free(pd_entries[i].value);
    pd_entries[i] = pd_entries[i + nmatches];
  }
  pd_entries.length(pd_entries.length() - nmatches);
}


void
ContextImpl::get_values(const char* start_scope, CORBA::Flags op_flags,
			const char* pattern, CORBA::NVList_out values_out)
{
  if( !pattern || !*pattern )
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_EmptyContextPattern,CORBA::COMPLETED_NO);

  ContextImpl* c = this;
  if( start_scope && *start_scope ){
    // find the starting scope
    while( !CORBA::is_nil(c) && strcasecmp(c->pd_name, start_scope) )
      c = (ContextImpl*)c->pd_parent;
    if( CORBA::is_nil(c) )
      OMNIORB_THROW(BAD_CONTEXT,
		    BAD_CONTEXT_StartingScopeNotFound, CORBA::COMPLETED_NO);
  }

  int wildcard = 0;
  if( pattern[strlen(pattern) - 1] == '*' )
    wildcard = 1;

  CORBA::NVList_ptr nvlist = new NVListImpl();

  try{
    add_values(c, op_flags, pattern, wildcard, nvlist);
  }
  catch(...){
    CORBA::release(nvlist);
    throw;
  }
  if( nvlist->count() == CORBA::ULong(0) ){
    CORBA::release(nvlist);
    OMNIORB_THROW(BAD_CONTEXT, BAD_CONTEXT_NoMatchingProperty,
		  CORBA::COMPLETED_NO);
  }else
    values_out = nvlist;

}


CORBA::Boolean
ContextImpl::NP_is_nil() const
{
  return 0;
}


CORBA::Context_ptr
ContextImpl::NP_duplicate()
{
  omni_tracedmutex_lock sync(pd_lock);
  pd_refCount++;
  return this;
}


void
ContextImpl::insert_single_consume(char* name, char* value)
{
  omni_tracedmutex_lock lock(pd_lock);

  // Binary search to determine insertion point
  CORBA::ULong count = pd_entries.length();
  CORBA::ULong bottom = 0;
  CORBA::ULong top = count;
  CORBA::Long match = -1;

  while( bottom < top ){
    CORBA::ULong i = (bottom + top) / 2;

    int cmp = strcasecmp(name, pd_entries[i].name);

    if( cmp < 0 )       top = i;
    else if( cmp > 0 )  bottom = (bottom == i) ? i + 1 : i;
    else{
      match = i;
      break;
    }
  }

  if( match >= 0 ){
    // Already has an entry - replace it.
    CORBA::string_free(pd_entries[match].value);
    CORBA::string_free(name);
    pd_entries[match].value = value;
    return;
  }

  if( count == pd_entries.maximum() )
    // Allocate new space in decent chunks.
    pd_entries.length(count * 6 / 5 + 1);

  pd_entries.length(count + 1);

  // <bottom> is the index where we want to insert this value.
  for( CORBA::ULong i = count; i > bottom; i-- )
    pd_entries[i] = pd_entries[i - 1];

  pd_entries[bottom].name = name;
  pd_entries[bottom].value = value;
}


void
ContextImpl::decrRefCount()
{
  CORBA::Boolean delete_me = 0;
  {
    omni_tracedmutex_lock sync(pd_lock);

    if( !pd_refCount ) {
      if( omniORB::traceLevel > 0 ) {
	omniORB::logger log;
	log <<
	  "omniORB: WARNING -- CORBA::release() was called too many times\n"
	  " for a CORBA::Context object - the object has already been\n"
	  " destroyed.\n";
      }
      return;
    }

    // We can only delete <this> if there are no dependent children.
    if( --pd_refCount == 0 && !pd_children )
      delete_me = 1;
  }

  if( delete_me )  delete this;
}


int
ContextImpl::matchPattern(const char* pattern, CORBA::ULong& bottom_out,
			  CORBA::ULong& top_out) const
{
  if( !pattern || !*pattern )
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_EmptyContextPattern,CORBA::COMPLETED_NO);

  size_t pat_len = strlen(pattern);
  int wildcard = 0;
  if( pattern[pat_len - 1] == '*' ){
    wildcard = 1;
    pat_len--;
  }

  CORBA::ULong count = pd_entries.length();
  CORBA::ULong bottom = 0;
  CORBA::ULong top = count;
  int match = 0;

  if (wildcard && pat_len == 0) {
    // Match everything
    bottom_out = bottom;
    top_out = top;
    return (top != bottom);
  }

  // Find a match (binary search).
  while( bottom < top ){
    CORBA::ULong i = (bottom + top) / 2;
    int cmp;

    if( wildcard )  cmp = ::strncasecmp(pattern, pd_entries[i].name, pat_len);
    else            cmp = ::strcasecmp(pattern, pd_entries[i].name);

    if( cmp < 0 )       top = i;
    else if( cmp > 0 )  bottom = (bottom == i) ? i + 1 : i;
    else{
      match = 1;
      bottom = i;
      break;
    }
  }

  if( !match )  return 0;

  if( wildcard ){
    top = bottom + 1;
    while( bottom > 0 &&
	   !strncasecmp(pattern, pd_entries[bottom - 1].name, pat_len) )
      bottom--;
    while( top < count &&
	   !strncasecmp(pattern, pd_entries[top].name, pat_len) )
      top++;
    bottom_out = bottom;
    top_out = top;
  }else{
    bottom_out = bottom;
    top_out = bottom + 1;
  }
  return 1;
}


void
ContextImpl::add_values(ContextImpl* c, CORBA::Flags op_flags,
			const char* pattern, int wildcard,
			CORBA::NVList_ptr val_list)
{
  if (!CORBA::NVList::PR_is_valid(val_list))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidNVList, CORBA::COMPLETED_NO);

  CORBA::ULong bottom, top;

  do{
    omni_tracedmutex_lock lock(c->pd_lock);

    if( c->matchPattern(pattern, bottom, top) ){
      for( CORBA::ULong i = bottom; i < top; i++ ){
	CORBA::ULong val_list_count = val_list->count();
	Entry* e = &(c->pd_entries[i]);

	// Search val_list to see if it already has entry for this name
	// (not very efficient).
	CORBA::ULong j;
	for( j = 0; j < val_list_count; j++ )
	  if( !strcasecmp(e->name, val_list->item(j)->name()) )
	    break;

	if( j == val_list_count ){
	  char* name = CORBA::string_dup(e->name);
	  CORBA::Any* value = new CORBA::Any();
	  *value <<= (const char*)e->value;
	  val_list->add_value_consume(name, value, CORBA::Flags(0));
	}
      }
      // pattern does not have a '*', so only one match required
      if( !wildcard )  return;
    }
    if( (op_flags & CORBA::CTX_RESTRICT_SCOPE) || CORBA::is_nil(c->pd_parent) )
      return;
    c = (ContextImpl*)c->pd_parent;
  }while( 1 );
}


void
ContextImpl::check_context_name(const char* cn)
{
  const unsigned char* n = (const unsigned char*)cn;

  if( !isalpha(*n++) )  OMNIORB_THROW(BAD_PARAM,
				      BAD_PARAM_InvalidContextName,
				      CORBA::COMPLETED_NO);
  while( isalnum(*n) || *n == '_' )  n++;

  if( *n != '\0' )  OMNIORB_THROW(BAD_PARAM,
				  BAD_PARAM_InvalidContextName,
				  CORBA::COMPLETED_NO);
}


void
ContextImpl::check_property_name(const char* n)
{
  do{
    if( !isalpha(*n++) )  OMNIORB_THROW(BAD_PARAM,
					BAD_PARAM_InvalidContextName,
					CORBA::COMPLETED_NO);
    while( isalnum(*n) || *n == '_' )  n++;
  }while( *n == '.' && (n++,1) );

  if( *n != '\0' )  OMNIORB_THROW(BAD_PARAM,
				  BAD_PARAM_InvalidContextName,
				  CORBA::COMPLETED_NO);
}


void
ContextImpl::loseChild(ContextImpl* child)
{
  CORBA::Boolean delete_me = 0;
  {
    // This lock guarentees that no other thread will be accessing the
    // list of children - as a context's list of siblings is managed by
    // its parent.
    omni_tracedmutex_lock sync(pd_lock);

    // Find the pointer to <child> in the list of dependents.
    ContextImpl** p = &pd_children;
    while( *p && *p != child )  p = &(*p)->pd_nextSibling;

    if( !*p )  throw omniORB::fatalException(__FILE__,__LINE__,
					     "ContextImpl::loseChild()");
    // Remove <child> from the list.
    *p = (*p)->pd_nextSibling;

    if( pd_refCount == 0 && !pd_children )
      delete_me = 1;
  }

  if( delete_me )  delete this;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilContext : public CORBA::Context, public omniTrackedObject {
public:
  virtual const char* context_name() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Context_ptr parent() const {
    _CORBA_invoked_nil_pseudo_ref();
    return _nil();
  }
  virtual void create_child(const char*, CORBA::Context_out) {
    _CORBA_invoked_nil_pseudo_ref();
    }
  virtual void set_one_value(const char*, const CORBA::Any&) {
    _CORBA_invoked_nil_pseudo_ref();
    }
  virtual void set_values(CORBA::NVList_ptr) {
    _CORBA_invoked_nil_pseudo_ref();
    }
  virtual void delete_values(const char*) {
    _CORBA_invoked_nil_pseudo_ref();
    }
  virtual void get_values(const char* start_scope,
				   CORBA::Flags op_flags,
				   const char* pattern,
				   CORBA::NVList_out values) {
    _CORBA_invoked_nil_pseudo_ref();
    }
  virtual CORBA::Boolean NP_is_nil() const {
    return 1;
  }
  virtual CORBA::Context_ptr NP_duplicate() {
    return _nil();
  }
};

OMNI_NAMESPACE_END(omni)

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
/////////////////////////////// Context //////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Context::~Context() { pd_magic = 0; }


CORBA::Context_ptr
CORBA::Context::_duplicate(Context_ptr p)
{
  if (!PR_is_valid(p))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidContext, CORBA::COMPLETED_NO);
  if( !CORBA::is_nil(p) ) {
    ContextImpl* c = (ContextImpl*) p;
    omni_tracedmutex_lock sync(c->pd_lock);
    c->pd_refCount++;
    return c;
  }
  else
    return _nil();
}


CORBA::Context_ptr
CORBA::Context::_nil()
{
  static omniNilContext* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniNilContext;
      registerTrackedObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

void
CORBA::Context::marshalContext(CORBA::Context_ptr ctxt,
			       const char*const* which,
			       int whichlen, cdrStream& s)
{
  if( CORBA::is_nil(ctxt) ) {
    CORBA::ULong(0) >>= s;
    return;
  }
  ContextImpl* c = (ContextImpl*) ctxt;

  _CORBA_Unbounded_Sequence_String seq;
  CORBA::ULong l = 0;

  do {
    omni_tracedmutex_lock sync(c->pd_lock);
    int i;
    CORBA::ULong top, bottom, j;

    for (i=0; i < whichlen; i++) {
      CORBA::ULong curr_len = l;

      if (c->matchPattern(which[i], bottom, top)) {
	for (; bottom < top; bottom++) {
	  ContextImpl::Entry* e = &(c->pd_entries[bottom]);

	  // Very inefficiently look to see if we've already added this name
	  for (j=0; j < curr_len; j+=2)
	    if (!strcasecmp(seq[j], e->name))
	      break;

	  if (j == curr_len) {
	    l += 2;
	    if (l > seq.maximum()) seq.length(l * 6 / 5 + 2);
	    seq.length(l);
	    seq[l-2] = (const char*)e->name;
	    seq[l-1] = (const char*)e->value;
	  }
	}
      }
    }
    if (CORBA::is_nil(c->pd_parent)) break;
    c = (ContextImpl*)c->pd_parent;
  } while(1);

  // Now marshal the sequence
  seq.length() >>= s;
  for (CORBA::ULong i=0; i < seq.length(); i++)
    s.marshalRawString(seq[i]);
}

CORBA::Context_ptr
CORBA::Context::unmarshalContext(cdrStream& s)
{
  CORBA::ULong nentries;
  nentries <<= s;
  if( nentries % 2 )  OMNIORB_THROW(MARSHAL,
				    MARSHAL_InvalidContextList,
				    CORBA::COMPLETED_MAYBE);
  nentries /= 2;

  ContextImpl* c = new ContextImpl("", CORBA::Context::_nil());

  try {
    for( CORBA::ULong i = 0; i < nentries; i++ ) {

      char* name  = s.unmarshalRawString();
      char* value = s.unmarshalRawString();
      c->insert_single_consume(name, value);
    }
  }
  catch(...) {
    delete c;
    throw;
  }

  return c;
}

CORBA::Context_ptr
CORBA::Context::filterContext(CORBA::Context_ptr ctxt,
			      const char*const* which,
			      int whichlen)
{
  ContextImpl* ret = new ContextImpl("", CORBA::Context::_nil());

  if (CORBA::is_nil(ctxt))
    return ret;

  if (!PR_is_valid(ctxt))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidContext, CORBA::COMPLETED_NO);

  ContextImpl* c = (ContextImpl*)ctxt;

  do {
    omni_tracedmutex_lock sync(c->pd_lock);
    int i;
    CORBA::ULong top, bottom, j, z1, z2;

    for (i=0; i < whichlen; i++) {

      if (c->matchPattern(which[i], bottom, top)) {
	for (; bottom < top; bottom++) {
	  ContextImpl::Entry* e = &(c->pd_entries[bottom]);

	  // See if already added
	  if (ret->matchPattern(e->name, z1, z2))
	    continue;

	  ret->insert_single_consume(CORBA::string_dup(e->name),
				     CORBA::string_dup(e->value));
	}
      }
    }
    if (CORBA::is_nil(c->pd_parent)) break;
    c = (ContextImpl*)c->pd_parent;
  } while(1);

  return ret;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////// CORBA ///////////////////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::release(CORBA::Context_ptr p)
{
  if( CORBA::Context::PR_is_valid(p) && !CORBA::is_nil(p) )
    ((ContextImpl*)p)->decrRefCount();
}


void
CORBA::ORB::get_default_context(CORBA::Context_out context_out)
{
  if( !default_context )
    default_context = new ContextImpl("default", CORBA::Context::_nil());

  context_out = CORBA::Context::_duplicate(default_context);
}
