// -*- Mode: C++; -*-
//                            Package   : omniORB2
// context.cc                 Created on: 9/1998
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
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

#include <context.h>
#include <pseudo.h>
#include <string.h>
#include <ctype.h>


#define INIT_MAX_SEQ_LENGTH  6


#ifdef _HAS_NOT_GOT_strcasecmp
extern int strcasecmp(const char *s1, const char *s2);
#endif
#ifdef _HAS_NOT_GOT_strncasecmp
extern int strncasecmp(const char *s1, const char *s2, size_t n);
#endif


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
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "ContextImpl::~ContextImpl()");

  // <pd_name> freed by String_var.
  // We don't own <pd_parent>.

  for( CORBA::ULong i = 0; i < pd_entries.length(); i++ ){
    if( pd_entries[i].name  )  CORBA::string_free(pd_entries[i].name);
    if( pd_entries[i].value )  CORBA::string_free(pd_entries[i].value);
  }

  if( !CORBA::is_nil(pd_parent) )
    ((ContextImpl*)pd_parent)->loseChild(this);
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


CORBA::Status
ContextImpl::create_child(const char* name, CORBA::Context_out out)
{
  out = CORBA::Context::_nil();
  // The c'tor checks that the name is legal.
  out = new ContextImpl(name, this);
  RETURN_CORBA_STATUS;
}


CORBA::Status
ContextImpl::set_one_value(const char* prop_name, const CORBA::Any& value)
{
  // Check the property name is valid.
  if( !prop_name )  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);
  check_property_name(prop_name);

  CORBA::String_var name(CORBA::string_dup(prop_name));

  char* strval;
  if( !(value >>= strval) )
    throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);

  insert_single_consume(name._retn(), strval);
  RETURN_CORBA_STATUS;
}


CORBA::Status
ContextImpl::set_values(CORBA::NVList_ptr values)
{
  if( CORBA::is_nil(values) )
    throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);

  CORBA::ULong count = values->count();

  for( CORBA::ULong i = 0; i < count; i++ ){
    CORBA::NamedValue_ptr nv = values->item(i);
    set_one_value(nv->name(), *nv->value());
  }
  RETURN_CORBA_STATUS;
}


CORBA::Status
ContextImpl::delete_values(const char* pattern)
{
  omni_mutex_lock lock(pd_lock);
  CORBA::ULong bottom, top;

  if( !matchPattern(pattern, bottom, top) )
    throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);

  CORBA::ULong nmatches = top - bottom;
  CORBA::ULong count = pd_entries.length();

  // delete entries, and move ones above down
  for( CORBA::ULong i = bottom; i  + nmatches < count; i++ ){
    CORBA::string_free(pd_entries[i].name);
    CORBA::string_free(pd_entries[i].value);
    pd_entries[i] = pd_entries[i + nmatches];
  }
  pd_entries.length(pd_entries.length() - nmatches);
  RETURN_CORBA_STATUS;
}


CORBA::Status
ContextImpl::get_values(const char* start_scope, CORBA::Flags op_flags,
			const char* pattern, CORBA::NVList_out values_out)
{
  ContextImpl* c = this;
  if( start_scope && *start_scope ){
    // find the starting scope
    while( !CORBA::is_nil(c) && strcasecmp(c->pd_name, start_scope) )
      c = (ContextImpl*)c->pd_parent;
    if( CORBA::is_nil(c) )
      throw CORBA::BAD_CONTEXT(0, CORBA::COMPLETED_NO);
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
    values_out = CORBA::NVList::_nil();
  }else
    values_out = nvlist;

  RETURN_CORBA_STATUS;
}


CORBA::Boolean
ContextImpl::NP_is_nil() const
{
  return 0;
}


CORBA::Context_ptr
ContextImpl::NP_duplicate()
{
  omni_mutex_lock sync(pd_lock);
  pd_refCount++;
  return this;
}

const char*
ContextImpl::lookup_single(const char* name) const
{
  CORBA::ULong bottom, top;
  if( !matchPattern(name, bottom, top) )  return (const char*)0;
  return pd_entries[bottom].value;
}


void
ContextImpl::insert_single_consume(char* name, char* value)
{
  omni_mutex_lock lock(pd_lock);

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
    omni_mutex_lock sync(pd_lock);

    if( !pd_refCount ) {
      if( omniORB::traceLevel > 0 ) {
	omniORB::log <<
	  "Warning: omniORB2 has detected that CORBA::release() was called\n"
	  " too many times for a CORBA::Context object - the object has\n"
	  " already been destroyed.\n";
	omniORB::log.flush();
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
    throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);

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

  // Find a match (binary search).
  while( bottom < top ){
    CORBA::ULong i = (bottom + top) / 2;
    int cmp;

    if( wildcard )  cmp = strncasecmp(pattern, pd_entries[i].name, pat_len);
    else            cmp = strcasecmp(pattern, pd_entries[i].name);

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
  CORBA::ULong bottom, top;
  omni_mutex_lock lock(c->pd_lock);

  do{
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
ContextImpl::check_context_name(const char* n)
{
  if( !isalpha(*n++) )  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);
  while( isalnum(*n) || *n == '_' )  n++;
  if( *n != '\0' )  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);
}


void
ContextImpl::check_property_name(const char* n)
{
  do{
    if( !isalpha(*n++) )  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);
    while( isalnum(*n) || *n == '_' )  n++;
  }while( *n == '.' && (n++,1) );

  if( *n != '\0' )  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);
}


void
ContextImpl::loseChild(ContextImpl* child)
{
  CORBA::Boolean delete_me = 0;
  {
    // This lock guarentees that no other thread will be accessing the
    // list of children - as a context's list of siblings is managed by
    // its parent.
    omni_mutex_lock sync(pd_lock);

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

class NilContext : public CORBA::Context {
public:
  virtual const char* context_name() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Context_ptr parent() const {
    _CORBA_invoked_nil_pseudo_ref();
    return _nil();
  }
  virtual CORBA::Status create_child(const char*, CORBA::Context_out) {
    _CORBA_invoked_nil_pseudo_ref();
    RETURN_CORBA_STATUS;
  }
  virtual CORBA::Status set_one_value(const char*, const CORBA::Any&) {
    _CORBA_invoked_nil_pseudo_ref();
    RETURN_CORBA_STATUS;
  }
  virtual CORBA::Status set_values(CORBA::NVList_ptr) {
    _CORBA_invoked_nil_pseudo_ref();
    RETURN_CORBA_STATUS;
  }
  virtual CORBA::Status delete_values(const char*) {
    _CORBA_invoked_nil_pseudo_ref();
    RETURN_CORBA_STATUS;
  }
  virtual CORBA::Status get_values(const char* start_scope,
				   CORBA::Flags op_flags,
				   const char* pattern,
				   CORBA::NVList_out values) {
    _CORBA_invoked_nil_pseudo_ref();
    RETURN_CORBA_STATUS;
  }
  virtual CORBA::Boolean NP_is_nil() const {
    return 1;
  }
  virtual CORBA::Context_ptr NP_duplicate() {
    return _nil();
  }
};

static NilContext _nilContext;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Context::~Context() {}


CORBA::Context_ptr
CORBA::Context::_duplicate(Context_ptr p)
{
  if( p )  return p->NP_duplicate();
  else     return _nil();
}


CORBA::Context_ptr
CORBA::Context::_nil()
{
  return &_nilContext;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::release(Context_ptr p)
{
  if( !p->NP_is_nil() )
    ((ContextImpl*)p)->decrRefCount();
}


static ContextImpl* default_context = 0;

CORBA::Status
CORBA::ORB::get_default_context(CORBA::Context_out context_out)
{
  if( !default_context )
    default_context = new ContextImpl("default", CORBA::Context::_nil());

  context_out = default_context->NP_duplicate();
  RETURN_CORBA_STATUS;
}
