// -*- Mode: C++; -*-
//                            Package   : omniORB2
// DynAnyP.cc                 Created on: 12/02/98
//                            Author    : Sai-Lai Lo (sll)
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
//      Internal implementation support for type DynAny


/*
   $Log$
   Revision 1.4  1998/08/26 11:12:22  sll
   Minor updates to remove warnings when compiled with standard C++ compilers.

   Revision 1.3  1998/08/25 18:52:39  sll
   Added sign-unsigned castings to keep gcc-2.7.2 and egcs happy.

   Revision 1.2  1998/08/14 13:46:01  sll
   Added pragma hdrstop to control pre-compile header if the compiler feature
   is available.

   Revision 1.1  1998/08/05 18:03:51  sll
   Initial revision

*/

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <tcParseEngine.h>
#include <dynAnyP.h>

#define CvtDynAnyP()     ((dynAnyP*)(NP_pd()))

omni_mutex dynAnyP::lock;
omni_mutex dynAnyP::refCountLock;

static
void internal_initialise_buffer(MemBufferedStream&,CORBA::TypeCode_ptr);

static
void internal_rewind_buffer(MemBufferedStream&,CORBA::TypeCode_ptr);

static
CORBA::ULong internal_buffer_getpos(MemBufferedStream&);

static
void internal_buffer_setpos(MemBufferedStream&,CORBA::ULong);

static
CORBA::ULong internal_read_seqlen_from_buffer(MemBufferedStream&,
					      CORBA::TypeCode_ptr);

static
void internal_write_seqlen_into_buffer(MemBufferedStream&,
                                       CORBA::TypeCode_ptr,CORBA::ULong);


dynAnyP::dynAnyP(CORBA::TypeCode_ptr tc)
{
  if (tc->kind() == CORBA::tk_alias)
    pd_tc = tc->NP_aliasExpand();
  else 
    pd_tc = CORBA::TypeCode::_duplicate(tc);
  pd_refcount = 1;
  pd_is_root = 1;
  switch (pd_tc->kind()) {
  case CORBA::tk_sequence:
    pd_componentTC.val = pd_tc->content_type();
    if (pd_componentTC.val->kind() == CORBA::tk_alias) {
      CORBA::TypeCode_ptr ntc = pd_componentTC.val->NP_aliasExpand();
      CORBA::release(pd_componentTC.val);
      pd_componentTC.val = ntc;
    }
    pd_total_components = 0;
    break;
  case CORBA::tk_struct:
  case CORBA::tk_except:
    pd_componentTC.seq = 0;
    pd_total_components = tc->member_count();
    break;
  case CORBA::tk_array:
    pd_componentTC.val = pd_tc->content_type();
    pd_total_components = tc->length();
    if (pd_componentTC.val->kind() == CORBA::tk_alias) {
      CORBA::TypeCode_ptr ntc = pd_componentTC.val->NP_aliasExpand();
      CORBA::release(pd_componentTC.val);
      pd_componentTC.val = ntc;
    }
    break;
  case CORBA::tk_union:
    pd_componentTC.seq = 0;
    pd_total_components = 2;
    break;
  default:
    pd_componentTC.val = pd_tc;
    pd_total_components = 1;
  }
  internal_initialise_buffer(pd_buf,tc);
  if (tc->kind() == CORBA::tk_sequence) {
    internal_write_seqlen_into_buffer(pd_buf,tc,pd_total_components);
  }
  internal_rewind_buffer(pd_buf,tc);
  pd_curr_startpos = internal_buffer_getpos(pd_buf);
  pd_curr_index = 0;
  pd_last_index = -1;
  pd_state = Ready;

  if (pd_tc->kind() == CORBA::tk_union) {
    // Set up this node for union type.
    // pd_components[0] contains the dynAny for the discriminator
    // pd_components[1] contains the dynAny for the member
    // If the discriminator is a enum, use dynAnyUD2 (a private type) to
    // instantiate the dynAny member.
    // Otherwise, use dynAnyUD1 (a private type) to instantiate the member.
    pd_componentTC.seq = new
                          _CORBA_Unbounded_Sequence<CORBA::TypeCode_member>;
    pd_componentTC.seq->length(2);
    (*pd_componentTC.seq)[0] = tc->discriminator_type();
    (*pd_componentTC.seq)[1] = CORBA::TypeCode::_nil();

    if ((*pd_componentTC.seq)[0]->kind() == CORBA::tk_alias) {
      (*pd_componentTC.seq)[0] = (*pd_componentTC.seq)[0]->NP_aliasExpand();
    }

    pd_components.length(2);

    dynAnyP* p = new dynAnyP((*pd_componentTC.seq)[0]);
    switch ((*pd_componentTC.seq)[0]->kind()) {
    case CORBA::tk_enum:
      pd_components[0] = (CORBA::DynAny_ptr)(new dynAnyUD2(p,this));
      break;
    default:
      pd_components[0] = (CORBA::DynAny_ptr)(new dynAnyUD1(p,this));
    }
  }
}


dynAnyP::~dynAnyP()
{
  switch (pd_tc->kind()) {
  case CORBA::tk_sequence:
  case CORBA::tk_array:
    CORBA::release(pd_componentTC.val);
    break;
  case CORBA::tk_struct:
  case CORBA::tk_except:
  case CORBA::tk_union:
    if (pd_componentTC.seq) delete pd_componentTC.seq;
    break;
  default:
    break;
  }
  delete pd_tc;
}

void
dynAnyP::totalComponents(CORBA::ULong v)
{
  if (tc()->kind() == CORBA::tk_sequence &&
      (tc()->length() == 0 || tc()->length() >= v))
    {
      // A sequence and within the bounds (if any) defined in the IDL
      pd_total_components = v;
      internal_write_seqlen_into_buffer(pd_buf,tc(),pd_total_components);

      if (pd_components.length() > (CORBA::ULong)pd_total_components) {
	// This sequence has been truncated.
	// pd_components may contain some nodes that has to be
	// deleted because the sequence has been truncated.
	CORBA::ULong index = pd_total_components;
	for (index = (index?index-1:0);
	     index < pd_components.length(); index++)
	  {
	    if (!CORBA::is_nil(pd_components[index])) {
	      dynAnyP* p = (dynAnyP*) pd_components[index]->NP_pd();
	      p->pd_is_root = 1;
	      pd_components[index] = CORBA::DynAny::_nil();
	    }
	  }
	pd_components.length(pd_total_components);
      }

      if (nComponentInBuffer() > (CORBA::ULong)pd_total_components) {
	// This sequence has been truncated. And some of the truncated
	// elements have already been written into the buffer.
	pd_last_index = pd_total_components - 1;
	if (currentComponentIndex() > pd_last_index) {
	  pd_curr_index = -1;
	  pd_state = Invalid;
	}
	// Create a node in pd_components. This make sure that
	// when the value is exported by toAny, the whole data
	// stream will be remarshalled, hence the truncated data in
	// the buffer would be filtered out.
	createComponent(0);
      }
    }
  else {
    throw omniORB::fatalException(__FILE__,__LINE__,
	"dynAnyP::totalComponent(CORBA::ULong) called either on a fixed size dynAny or on a bounded sequence that has a lower bound than the requested value.");
  }
}


CORBA::ULong
dynAnyP::MaxComponents() const
{
  if (tc()->kind() == CORBA::tk_sequence)
    return tc()->length();
  else
    return totalComponents();
}



CORBA::DynAny_ptr
dynAnyP::currentComponent()
{
  if (currentComponentIndex() < 0 || CORBA::is_nil(currentComponentTC())) {
    return CORBA::DynAny::_nil();
  }

  createComponent((CORBA::ULong)currentComponentIndex());
  CORBA::DynAny_ptr retval = pd_components[(CORBA::ULong)
					  currentComponentIndex()];
  ((dynAnyP*)(retval->NP_pd()))->incrRefCount();
  return retval;
}


void
dynAnyP::currentComponentFromBasetype(CORBA::TCKind k,Bval& v)
{
  if (currentComponentTCKind() != k) {
    throw CORBA::DynAny::InvalidValue();
  }

  // Now Check where the value is stored in this order:
  // 1. check pd_components to see if a dynAny node has
  //    been created for this component.
  // 2. check if the value can be appended to the end of this node's
  //    buffer.
  //
  if (pd_components.length() > (CORBA::ULong)currentComponentIndex() &&
      ! CORBA::is_nil(pd_components[currentComponentIndex()]))
    {
      dynAnyP* p = (dynAnyP*)pd_components[currentComponentIndex()]->NP_pd();
      if (p->totalComponents() != 1) {
	throw CORBA::DynAny::InvalidValue();
      }
      p->currentComponentFromBasetype(k,v);
      return;
    }


  if (beginWriteComponent())
    {
      // Append value to the end of this buffer
      switch(k) {
      case CORBA::tk_short:
	v.sv >>= pd_buf;
	break;
      case CORBA::tk_long:
	v.lv >>= pd_buf;
	break;
      case CORBA::tk_ushort:
	v.usv >>= pd_buf;
	break;
      case CORBA::tk_ulong:
	v.ulv >>= pd_buf;
	break;
#ifndef NO_FLOAT
#ifndef USING_PROXY_FLOAT
      case CORBA::tk_float:
	v.fv >>= pd_buf;
	break;
      case CORBA::tk_double:
	v.dv >>= pd_buf;
	break;
#else
      case CORBA::tk_float: {
	CORBA::Float tmp(v.fv);
	tmp >>=3D pd_buf;
      } 
      break;
      case CORBA::tk_double: {
	CORBA::Double tmp(v.dv);
	tmp >>=3D pd_buf;
      } 
      break;
#endif
#endif
      case CORBA::tk_boolean:
	v.bv >>= pd_buf;
	break;
      case CORBA::tk_char:
	v.cv >>= pd_buf;
	break;
      case CORBA::tk_octet:
	v.ov >>= pd_buf;
	break;
      case CORBA::tk_any:
	*v.anyv >>= pd_buf;
	break;
      case CORBA::tk_TypeCode:
	*v.tcv >>= pd_buf;
	break;
      case CORBA::tk_objref:
	CORBA::Object::marshalObjRef(v.objv,pd_buf);
	break;
      case CORBA::tk_enum:
	v.ulv >>= pd_buf;
	break;
      case CORBA::tk_string:
	{
	  CORBA::TypeCode_ptr tc = currentComponentTC();
	  CORBA::ULong l = strlen(v.stiv);
	  if (tc->length() != 0 && tc->length() < l)
	    throw CORBA::DynAny::InvalidValue();
	  l++;
	  l >>= pd_buf;
	  pd_buf.put_char_array((CORBA::Char*)v.stiv,l);
	  break;
	}
      default:
	throw CORBA::DynAny::InvalidValue();
      }

      endWriteComponent();
    }
  else {
    // We cannot append to the buffer, either:
    //  1. overwriting (currentComponentIndex() < nComponentInBuffer()) or
    //  2. have skipped some components
    //            (currentComponentIndex() > nComponentInBuffer())
    // Either way, create a dynAny node in pd_components.
    createComponent((CORBA::ULong)currentComponentIndex());
    dynAnyP* p = (dynAnyP*)pd_components[currentComponentIndex()]->NP_pd();
    if (p->totalComponents() != 1) {
      throw CORBA::DynAny::InvalidValue();
    }
    p->currentComponentFromBasetype(k,v);
  }
}

void
dynAnyP::currentComponentToBasetype(CORBA::TCKind k,Bval& v)
{
  if (currentComponentTCKind() != k) {
    throw CORBA::DynAny::TypeMismatch();
  }

  // Check where the value is stored in this order:
  // 1. check pd_components to see if a dynAny node has
  //    been created for this component.
  // 2. check if the buffer of this node contains the value.
  //
  if (pd_components.length() > (CORBA::ULong)currentComponentIndex() &&
      ! CORBA::is_nil(pd_components[currentComponentIndex()]))
    {
      dynAnyP* p = (dynAnyP*)pd_components[currentComponentIndex()]->NP_pd();
      if (p->totalComponents() != 1) {
	throw CORBA::DynAny::TypeMismatch();
      }
      p->currentComponentToBasetype(k,v);
      return;
    }

  if (beginReadComponent())
    {
      // The value is in the buffer
      switch(k) {
      case CORBA::tk_short:
	v.sv <<= pd_buf;
	break;
      case CORBA::tk_long:
	v.lv <<= pd_buf;
	break;
      case CORBA::tk_ushort:
	v.usv <<= pd_buf;
	break;
      case CORBA::tk_ulong:
	v.ulv <<= pd_buf;
	break;
#ifndef NO_FLOAT
#ifndef USING_PROXY_FLOAT
      case CORBA::tk_float:
	v.fv <<= pd_buf;
	break;
      case CORBA::tk_double:
	v.dv <<= pd_buf;
	break;
#else
      case CORBA::tk_float: {
	CORBA::Float tmp;
	tmp <<=3D pd_buf;
	v.fv =3D tmp;
      } 
      break;
      case CORBA::tk_double: {
	CORBA::Double tmp;
	tmp <<=3D pd_buf;
	v.dv =3D tmp;
      } 
      break;
#endif
#endif
      case CORBA::tk_boolean:
	v.bv <<= pd_buf;
	break;
      case CORBA::tk_char:
	v.cv <<= pd_buf;
	break;
      case CORBA::tk_octet:
	v.ov <<= pd_buf;
	break;
      case CORBA::tk_any:
	v.anyv = new CORBA::Any;
	*v.anyv <<= pd_buf;
	break;
      case CORBA::tk_TypeCode:
	v.tcv = new CORBA::TypeCode(CORBA::tk_null);
	*v.tcv <<= pd_buf;
	break;
      case CORBA::tk_objref:
	v.objv = CORBA::Object::unmarshalObjRef(pd_buf);
	break;
      case CORBA::tk_enum:
	v.ulv <<= pd_buf;
	break;
      case CORBA::tk_string:
	{
	  CORBA::ULong l;
	  l <<= pd_buf;
	  CORBA::TypeCode_ptr tc = currentComponentTC();
	  if (l && tc->length() != 0 && tc->length() < l-1)
	    throw CORBA::DynAny::TypeMismatch();
	  if (l) {
	    v.stov = CORBA::string_alloc(l-1);
	    pd_buf.get_char_array((CORBA::Char*)v.stov,l);
	    v.stov[l-1] = '\0';
	  }
	  else {
	    v.stov = CORBA::string_alloc(0);
	    v.stov[0] = '\0';
	  }
	  break;
	}
      default:
	throw CORBA::DynAny::TypeMismatch();
      }
      endReadComponent();
    }
  else
    {
      // Reach here if and only if no valid value can be extracted.
      throw CORBA::DynAny::TypeMismatch();
    }
}

void
dynAnyP::currentComponentFromAny(CORBA::Any& v)
{
  if (currentComponentIndex() < 0) {
    throw CORBA::DynAny::InvalidValue();
  }

  CORBA::TypeCode_ptr dtc = currentComponentTC();
  CORBA::TypeCode_var stc = v.type();
  if (!stc->NP_expandEqual(dtc,1))
    throw CORBA::DynAny::InvalidValue();

  // Check where the value is stored in this order:
  // 1. check pd_components to see if a dynAny node has
  //    been created for this component.
  // 2. check if the value can be appended to the end of this node's
  //    buffer.
  //
  if (pd_components.length() > (CORBA::ULong)currentComponentIndex() &&
      ! CORBA::is_nil(pd_components[currentComponentIndex()]))
    {
      dynAnyP* p = (dynAnyP*)pd_components[currentComponentIndex()]->NP_pd();
      p->fromAny(v);
      return;
    }

  if (beginWriteComponent())
    {
      // Append value to the end of this buffer
      MemBufferedStream src;
      v.NP_getBuffer(src);
      tcParseEngine tpe(dtc,&src);
      (void) tpe.parse(pd_buf);
      endWriteComponent();
    }
  else
    {
      // We cannot append to the buffer, either:
      //  1. overwriting (currentComponentIndex() < nComponentInBuffer) or
      //  2. have skipped some components
      //            (currentComponentIndex() > nComponentInBuffer)
      // Either way, create a dynAny node in pd_components.
      createComponent((CORBA::ULong)currentComponentIndex());
      dynAnyP* p = (dynAnyP*)pd_components[currentComponentIndex()]->NP_pd();
      p->fromAny(v);
    }
}

void
dynAnyP::currentComponentToAny(CORBA::Any& v)
{
  if (currentComponentIndex() < 0) {
    throw CORBA::DynAny::TypeMismatch();
  }

  CORBA::TypeCode_ptr tc = currentComponentTC();

  // Check where the value is stored in this order:
  // 1. check pd_components to see if a dynAny node has
  //    been created for this component.
  // 2. check if the buffer of this node contains the value.
  //
  if (pd_components.length() > (CORBA::ULong)currentComponentIndex() &&
      ! CORBA::is_nil(pd_components[currentComponentIndex()]))
    {
      dynAnyP* p = (dynAnyP*)pd_components[currentComponentIndex()]->NP_pd();
      p->toAny(v);
      return;
    }

  MemBufferedStream m;
  internal_initialise_buffer(m,tc);

  if (beginReadComponent())
    {
      // The value is in the buffer

      tcParseEngine  tpe(tc,&pd_buf);

      (void) tpe.parse(m);

      v.NP_replaceData(tc,m);

      endReadComponent();
    }
  else
    {
      // Reach here if and only if no valid value can be extracted.
      throw CORBA::DynAny::TypeMismatch();
    }

}


void
dynAnyP::fromAny(const CORBA::Any& v)
{
  CORBA::TypeCode_var stc = v.type();

  if (!stc->NP_expandEqual(tc(),1))
    throw CORBA::DynAny::Invalid();

  try {
    CORBA::ULong saved_curr = currentComponentIndex();
    MemBufferedStream src;
    v.NP_getBuffer(src);
    tcParseEngine tpe(tc(),&src);
    internal_initialise_buffer(pd_buf,tc());
    (void) tpe.parse(pd_buf);
    if (tc()->kind() != CORBA::tk_sequence) {
      pd_last_index = totalComponents() - 1;
    }
    else {
      // The length of the sequence is in the buffer.
      pd_last_index = (int) internal_read_seqlen_from_buffer(pd_buf,tc()) - 1;
      totalComponents(pd_last_index+1);
    }

    pd_state = Invalid;
    if (pd_components.length() != 0) {
      // A non-nil component may exist in pd_components.
      // Scan the list and reinitialise any of these components with
      // the value in the buffer.
      nthComponent(0);
      for (CORBA::ULong index = 0; index < pd_components.length(); index++) {
	if (!CORBA::is_nil(pd_components[index]))
	  {
	    CORBA::Any tmpany;
	    MemBufferedStream m;
	    internal_initialise_buffer(m,nthComponentTC(index));

	    if (beginReadComponent())
	      {
		// The value is in the buffer

		tcParseEngine  tpe(nthComponentTC(index),&pd_buf);

		(void) tpe.parse(m);

		tmpany.NP_replaceData(nthComponentTC(index),m);

		endReadComponent();
	      }
	    else {
	      throw CORBA::DynAny::Invalid();
	    }
	    (pd_components[index])->NP_real_from_any(tmpany);
	}
	nextComponent();
      }
      if (tc()->kind() == CORBA::tk_union) {
	// After we have extract all values from the buffer, we
	// invalidate the buffer content. All extraction or insertion
	// would be done through the nodes in pd_components.
	pd_last_index = -1;
      }
    }
    nthComponent(saved_curr);
  }
  catch (...) {
    // Reinitialise this node to indicate that it contains no valid data
    pd_curr_index = pd_last_index = -1;
    pd_state = Invalid;
    // XXX there may still be some valid dynAny in pd_components, these
    // components would have old values. Should really invalidate them as
    // well.
    throw CORBA::DynAny::Invalid();
  }
}


void
dynAnyP::toStream(MemBufferedStream& m)
{
  CORBA::ULong idx;

  if (pd_components.length() < totalComponents() &&
      nComponentInBuffer() != totalComponents())
    {
      throw CORBA::DynAny::Invalid();
    }

  if (tc()->kind() == CORBA::tk_sequence) {
    // first marshal the length of the sequence.
    CORBA::ULong v = pd_total_components;
    v >>= m;
  }

  if (tc()->kind() != CORBA::tk_union) {

    if (nComponentInBuffer() > 0) {
      // Goes through all the components that are in the buffer
      MemBufferedStream src(pd_buf);
      internal_rewind_buffer(src,tc());

      for (idx = 0; idx < nComponentInBuffer(); idx++) {

	CORBA::TypeCode_ptr tc = nthComponentTC((CORBA::ULong)idx);

	if (pd_components.length()>idx &&
	    ! CORBA::is_nil(pd_components[(CORBA::ULong)idx]))
	  {
	    // This component may have been overwritten, get its value
	    // from pd_components.
	    dynAnyP* p = (dynAnyP*)pd_components[(CORBA::ULong)idx]->NP_pd();
	    p->toStream(m);

	    // Advance the pointer in src by extracting the value into
	    // a dummy stream.
	    MemBufferedStream dummy;
	    tcParseEngine tpe(tc,&src);
	    (void) tpe.parse(dummy);

	  }
	else {
	  // Extract the value from src.
	  tcParseEngine tpe(tc,&src);
	  (void) tpe.parse(m);
	}
      }
    }

    // Now goes through all the remaining components (if there is any).
    // They are all in pd_components
    for (idx = nComponentInBuffer(); idx < totalComponents(); idx++) {
      if (CORBA::is_nil(pd_components[(CORBA::ULong)idx]))
	throw CORBA::DynAny::Invalid();
      dynAnyP* p = (dynAnyP*)pd_components[(CORBA::ULong)idx]->NP_pd();
      p->toStream(m);
    }

  }
  else {

    // Deal with union differently.
    // The value of the union is always in pd_components[0] and
    // pd_components[1].
    //
    // One complication is that a union may have an implicit default member
    // if it does not have a default case and not all permissible values of
    // the union discriminant are listed. Under this condition the union's
    // value is composed solely of the discriminator value. In other words,
    // pd_components[1] may or may not have to be marshalled.

    ((dynAnyP*)(pd_components[0]->NP_pd()))->toStream(m);

    if (pd_components[0]->NP_nodetype() == CORBA::DynAny::dt_any) {
      dynAnyUD1* p = (dynAnyUD1*) ((CORBA::DynAny_ptr)pd_components[0]);
      if (p->implicit_default_member()) {
	return;
      }
    }
    else {
      dynAnyUD2* p = (dynAnyUD2*) ((CORBA::DynAny_ptr)pd_components[0]);
      if (p->implicit_default_member()) {
	return;
      }
    }
    // Reach here only if we have to marshal the branch member
    if (CORBA::is_nil(pd_components[1]))
      throw CORBA::DynAny::Invalid();
    ((dynAnyP*)(pd_components[1]->NP_pd()))->toStream(m);
  }
}

void
dynAnyP::toAny(CORBA::Any& v)
{
  if (nComponentInBuffer()==totalComponents() && pd_components.length()==0) {
    // everything is in the buffer.
    v.NP_replaceData(tc(),pd_buf);
  }
  else {
    MemBufferedStream m;
    internal_initialise_buffer(m,tc());
    toStream(m);
    v.NP_replaceData(tc(),m);
  }
}



CORBA::TypeCode_ptr
dynAnyP::nthComponentTC(CORBA::ULong index)
{
  if (index >= (CORBA::ULong)pd_total_components)
    return CORBA::TypeCode::_nil();

  switch (pd_tc->kind()) {

  case CORBA::tk_struct:
  case CORBA::tk_except:
  case CORBA::tk_union:
    if (!pd_componentTC.seq)
      pd_componentTC.seq = new
                           _CORBA_Unbounded_Sequence<CORBA::TypeCode_member>;
    if (pd_componentTC.seq->length() <= index) {
      pd_componentTC.seq->length(index+1);
    }
    if (CORBA::is_nil((*pd_componentTC.seq)[index])) {
      if (pd_tc->kind() != CORBA::tk_union) {
	(*pd_componentTC.seq)[index] = pd_tc->member_type(index);
	if ((*pd_componentTC.seq)[index]->kind() == CORBA::tk_alias) {
	  (*pd_componentTC.seq)[index] = (*pd_componentTC.seq)[index]
	                                      ->NP_aliasExpand();
	}
      }
      else {
	// Union member typecodes are initialised explicit as a side effect
	// of the DynUnion operations.
      }
    }
    return (*pd_componentTC.seq)[index];

  default:
    return pd_componentTC.val;
  }

}


void
dynAnyP::createComponent(CORBA::ULong index,CORBA::Boolean replace,
			 CORBA::TypeCode_ptr newtc)
{
  if (pd_components.length() <= index)
    pd_components.length(index+1);

  if (! CORBA::is_nil(pd_components[index]))
    {
      if (replace) {
	// detach the component before creating a new one.
	dynAnyP* p = (dynAnyP*) pd_components[index]->NP_pd();
	p->pd_is_root = 1;
	// Now as a new component is assigned, the old one is released.
	// If the reference count on the node is zero, it will be
	// deleted.
      }
      else {
	// component already exists and do not replace
	return;
      }
    }

  if (replace && tc()->kind() == CORBA::tk_union && index == 1) {
    (*pd_componentTC.seq)[1] = newtc;
    if (CORBA::is_nil(newtc)) {
      pd_components[1] == CORBA::DynAny::_nil();
      return;
    }
    else if ((*pd_componentTC.seq)[1]->kind() == CORBA::tk_alias) {
      (*pd_componentTC.seq)[1] = (*pd_componentTC.seq)[1]->NP_aliasExpand();
    }
  }

  CORBA::Any v;

  if (index < nComponentInBuffer()) {
    // There is a value for the component in the buffer, copy it
    int save_curr = currentComponentIndex();
    try {
      nthComponent(index);
      currentComponentToAny(v);
    }
    catch (...) {
    }
    if (save_curr >= 0) {
      nthComponent(save_curr);
    }
    else {
      pd_curr_index = -1;
      pd_state = Invalid;
    }
  }

  dynAnyP* p = new dynAnyP(nthComponentTC(index));

  p->pd_is_root = 0;           // This is not a root

  switch (nthComponentTC(index)->kind()) {
  case CORBA::tk_enum:
    pd_components[index]= new CORBA::DynEnum(p);
    break;
  case CORBA::tk_struct:
  case CORBA::tk_except:
    pd_components[index]= new CORBA::DynStruct(p);
    break;
  case CORBA::tk_union:
    pd_components[index]= new CORBA::DynUnion(p);
    break;
  case CORBA::tk_sequence:
    pd_components[index]= new CORBA::DynSequence(p);
    break;
  case CORBA::tk_array:
    pd_components[index]= new CORBA::DynArray(p);
    break;
#if 0
  case CORBA::tk_fixed:
    pd_components[index]= new CORBA::DynFixed(p);
    break;
#endif
  default:
    pd_components[index]= new CORBA::DynAny(p,CORBA::DynAny::dt_any);
  }

  if (index < nComponentInBuffer()) {
    // Now put the copied value into the newly created node
    p->fromAny(v);
  }
}

/*
   The following diagram records the state transition causes
   by the following methods:

         beginReadComponent()
         endReadComponent()
         beginWriteComponent()
         endWriteComponent()
         nextComponent()

     n denotes the current value of pd_curr_index
     m denotes the current value of pd_last_index

   For clarity the Ready state is subdivided into three states: Ready_1,
   Ready_2, Ready_3 depending on the value of n and m.

   Essentially, writing into the buffer can only be done by appending
   to the end of the buffer, i.e. when n == m+1. Reading data from the
   buffer can occur as long as n <= m.

   The transitions from any of the states to Invalid by nextComponent()
   are not shown. This transition occurs when the new n >= totalComponents()

   Any other state transition not shown is invalid. For example, calling
   endWriteComponent() when the current state is WriteCompleted is invalid
   and would cause a omniORB::fatalException to be raised.



   +-----------+
   |           |
   |  Ready_3  |
   |  (n>m+1)  |
   |           |          +-------------+
   +-----------+          |             |
     ^  ^                 | Ready_1     |
     |  |     +---------->| (n <= m)    +---------------+
     |  |     |           |             |  		|
     |  |     |           +-------------+  		|
  +--+  |     |                 	             beginReadComponent()
  |    	|     |	       	       	       	       	       	|
  |     |     |                 	                |
  |  nextComponent()                                    | beginReadComponent()
  |     |     |(n = n+1)                                |   +-----------+
  |     |<----|------+          	                |   |   (A)     |
  |     |     |      |          nextComponent()         |   |	        |
  |     |     |<-----+-------------------------------+  |   |	        |
  |     |     |                                      |  V   V           |
  |   +-+-----+-------+                           +--+-------------+	|
  |   |               |       endReadComponent()  |                |	|
  |   | ReadCompleted |<--------------------------| ReadInProgress +----+
  |   |               +-------------------------->|                |
  |   +--+---------+--+      beginReadComponent() +----------------+
  |      |(A)^     |	           		      	         ^
  |      +---+     +------------+  		      	         |
  |                             | nextComponent() (n=n+1)        |
  |nextComponent()              V		      	         |
  | (n=n+1)               +-------------+               beginReadComponent()
  |         +------------>|    	        |	      	         |
  |         |       +-----+  Ready_2    |	      	         |
  |         |       |     |  (n == m+1) |<-----------+ 	         |
  | beginReadCompon.|     |             |            |           |
  |         |       |     +-------------+	     |	         |
  |         |  	 beginWriteComponent()		nextComponent()	 |
  |         |  	    |                                | 	n=n+1    |
  |         |       |                                |           |
  |         |       V                                |           |
  |    +----+------------+      endWriteComponent() ++-----------+---+
  +----+                 +------------------------->|                |
       | WriteInProgress |      m = m + 1           | WriteCompleted |
    +--+                 |                          |                |
    |  +-----------------+                          +-----+----------+
    |     ^     					  |   ^
    +-----+     	                                  +---+
      (A)       					   (A)


     A = beginWriteComponent

*/


CORBA::Boolean
dynAnyP::beginReadComponent()
{
  switch (pd_state)
    {
    case WriteCompleted:
    case WriteInProgress:
    case ReadInProgress:
    case ReadCompleted:
      pd_state = Ready;
      // falls through
    case Ready:

      if (currentComponentIndex() >= (int)nComponentInBuffer())
	return 0;

      // Make sure that the buffer pointer is correct
      if ((int)internal_buffer_getpos(pd_buf) != pd_curr_startpos) {
	internal_buffer_setpos(pd_buf,pd_curr_startpos);
      }

      pd_state = ReadInProgress;
      return 1;

    case Invalid:
      return 0;
    }
   return 0;
}

void
dynAnyP::endReadComponent()
{
  switch (pd_state)
    {
    case ReadInProgress:
      pd_state = ReadCompleted;
      break;
    default:
      throw omniORB::fatalException(__FILE__,__LINE__,
	"dynAnyP::endReadComponent: called in invalid state.");
    }
}

CORBA::Boolean
dynAnyP::beginWriteComponent()
{
  switch (pd_state)
    {
    case ReadInProgress:
    case ReadCompleted:
    case WriteCompleted:
    case WriteInProgress:
    case Ready:

      if (totalComponents() == 1) {
	// This node only have one component, we can overwrite the
	// previous value in the buffer.
	internal_initialise_buffer(pd_buf,tc());
	pd_last_index = -1;
      }
      else {
       if (currentComponentIndex() != (int)nComponentInBuffer())
	 return 0;

       // Make sure that the buffer pointer is correct
       if ((int)pd_buf.WrMessageAlreadyWritten() != pd_curr_startpos) {
	 // A previous write has been done for this component
	 // Since we cannot remove things that have been written,
	 // the component's value can only be write into pd_components.
	 return 0;
       }
      }
      pd_state = WriteInProgress;
      return 1;

    case Invalid:
      return 0;
    }
   return 0;
}

void
dynAnyP::endWriteComponent()
{
  switch (pd_state)
    {
    case WriteInProgress:
      pd_state = WriteCompleted;
      pd_last_index++;
      break;
    default:
      throw omniORB::fatalException(__FILE__,__LINE__,
	"dynAnyP::endWriteComponent: called in invalid state.");
    }
}

CORBA::Boolean
dynAnyP::nextComponent()
{
  switch (pd_state)
    {
    case ReadInProgress:
      // Make sure that the pointer is correct and is the same
      // state as when it is in dynAnyP::Ready state
      if ((int)internal_buffer_getpos(pd_buf) != pd_curr_startpos) {
	internal_buffer_setpos(pd_buf,pd_curr_startpos);
      }

      // falls through
    case Ready:
      if (pd_curr_index <= pd_last_index) {
	// Skip the value of this component in the buffer
	MemBufferedStream dummy;
	tcParseEngine  tpe(currentComponentTC(),&pd_buf);
        (void) tpe.parse(dummy);
      }
      // falls through
    case ReadCompleted:
      pd_curr_index++;
      if (pd_curr_index >= (int)totalComponents()) {
	pd_curr_index = -1;
	pd_state = Invalid;
	return 0;
      }
      // Update the pointer to the buffer
      pd_curr_startpos = internal_buffer_getpos(pd_buf);

      pd_state = Ready;
      break;

    case WriteInProgress:
      // A previous write has not been completed properly.
      // The next beginWriteComponent() will detect this.
      pd_curr_index++;
      if (pd_curr_index >= (int)totalComponents()) {
	pd_curr_index = -1;
	pd_state = Invalid;
	return 0;
      }
      pd_state = Ready;
      break;

    case WriteCompleted:
      pd_curr_index++;
      if (pd_curr_index >= (int)totalComponents()) {
	pd_curr_index = -1;
	pd_state = Invalid;
	return 0;
      }
      pd_curr_startpos = pd_buf.WrMessageAlreadyWritten();
      pd_state = Ready;
      break;

    case Invalid:
      return 0;
    }
  return 1;
}


CORBA::Boolean
dynAnyP::nthComponent(CORBA::ULong index)
{
  if (index >= totalComponents())
    return 0;

  switch (pd_state)
    {
    case ReadCompleted:
    case WriteCompleted:
    case ReadInProgress:
    case WriteInProgress:
      (void) nextComponent();
      return nthComponent(index);

    case Ready:
      if (pd_curr_index <= (int)index) {
	for (int count=pd_curr_index; count < (int) index; count++)
	  nextComponent();
	return 1;
      }
      // falls through, let the next code block to skip to
      // the appropriate index.
    case Invalid:
      pd_curr_index = 0;

      // reset the pointer to the beginning of the buffer.
      internal_rewind_buffer(pd_buf,tc());
      pd_curr_startpos = internal_buffer_getpos(pd_buf);

      pd_state = Ready;

      for (int count=0; count < (int) index; count++)
	nextComponent();

      break;
    }
  return 1;
}


void
dynAnyP::incrRefCount()
{
  omni_mutex_lock sync(dynAnyP::refCountLock);
  pd_refcount++;
}

void
dynAnyP::decrRefCount(CORBA::DynAny_ptr dp)
{
  dynAnyP::refCountLock.lock();
  if (pd_refcount > 0)
    pd_refcount--;

  if (pd_refcount == 0 && pd_is_root) {
    // This is the root node and there is no more reference to this
    // node. Can delete this node.
    dynAnyP::refCountLock.unlock();
    dispose(dp);
  }
  else {
    dynAnyP::refCountLock.unlock();
  }
  // Note: if pd_is_root == 0, this is not the root node. dispose() is
  // not called. This node will be deleted when dispose() is called on the
  // root node which contains this node as part of its pd_components.
}

void
dynAnyP::dispose(CORBA::DynAny_ptr dp)
{
  dynAnyP::lock.lock();
  CORBA::ULong total = pd_components.length();

  for (CORBA::ULong index = 0; index < total; index++) {
    if (!CORBA::is_nil(pd_components[index])) {
      dynAnyP* p = (dynAnyP*) pd_components[index]->NP_pd();
      p->pd_is_root = 1;
      // Let the dtor of pd_components to call decrRefCount() on this
      // node.
    }
  }
  dynAnyP::lock.unlock();
  delete this;
  delete dp;
}


static int internal_union_value_to_index(CORBA::TypeCode_ptr tc,
					 dynAnyP::Bval v, CORBA::TCKind k);

static void internal_union_default_value(CORBA::TypeCode_ptr tc,
					 dynAnyP::Bval& v);

static CORBA::Boolean internal_union_no_default_value(CORBA::TypeCode_ptr tc);

void
dynAnyUD1::NP_real_from_any(const CORBA::Any& value)
{
  dynAnyP::Bval v;
  v.anyv = (CORBA::Any*) &value;
  int index = internal_union_value_to_index(pd_parent->tc(),v,
					    CORBA::tk_any);
  CvtDynAnyP()->fromAny(value);
  set_member(index,0);
}

void
dynAnyUD1::insert_boolean(CORBA::Boolean value)
{
  dynAnyP::Bval v;
  v.bv = value;
  int index = internal_union_value_to_index(pd_parent->tc(),v,
					    CORBA::tk_boolean);
  {
    // too bad we have to inline the body of CORBA::DynAny::insert_boolean
    // here instead of just calling it because we have to interlock
    // the insertion with set_member.
    omni_mutex_lock sync(dynAnyP::lock);
    if (CvtDynAnyP()->currentComponentIndex() >= 0)
      {
	dynAnyP::Bval v;
	v.bv = value;
	CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_boolean,v);
	if (NP_nodetype() != dt_any)
	  (void) CvtDynAnyP()->nextComponent();
      }
    else
      throw CORBA::DynAny::InvalidValue();
    set_member(index,0);
  }
}

void
dynAnyUD1::insert_octet(CORBA::Octet value)
{
  dynAnyP::Bval v;
  v.ov = value;
  int index = internal_union_value_to_index(pd_parent->tc(),v,
					    CORBA::tk_octet);
  {
    // too bad we have to inline the body of CORBA::DynAny::insert_octet
    // here instead of just calling it because we have to interlock
    // the insertion with set_member.
    omni_mutex_lock sync(dynAnyP::lock);
    if (CvtDynAnyP()->currentComponentIndex() >= 0)
      {
	dynAnyP::Bval v;
	v.ov = value;
	CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_octet,v);
	if (NP_nodetype() != dt_any)
	  (void) CvtDynAnyP()->nextComponent();
      }
    else
      throw CORBA::DynAny::InvalidValue();
    set_member(index,0);
  }
}

void
dynAnyUD1::insert_char(CORBA::Char value)
{
  dynAnyP::Bval v;
  v.cv = value;
  int index = internal_union_value_to_index(pd_parent->tc(),v,
					    CORBA::tk_char);
  {
    // too bad we have to inline the body of CORBA::DynAny::insert_char
    // here instead of just calling it because we have to interlock
    // the insertion with set_member.
    omni_mutex_lock sync(dynAnyP::lock);
    if (CvtDynAnyP()->currentComponentIndex() >= 0)
      {
	dynAnyP::Bval v;
	v.cv = value;
	CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_char,v);
	if (NP_nodetype() != dt_any)
	  (void) CvtDynAnyP()->nextComponent();
      }
    else
      throw CORBA::DynAny::InvalidValue();
    set_member(index,0);
  }
}

void
dynAnyUD1::insert_short(CORBA::Short value)
{
  dynAnyP::Bval v;
  v.sv = value;
  int index = internal_union_value_to_index(pd_parent->tc(),v,
					    CORBA::tk_short);
  {
    // too bad we have to inline the body of CORBA::DynAny::insert_short
    // here instead of just calling it because we have to interlock
    // the insertion with set_member.
    omni_mutex_lock sync(dynAnyP::lock);
    if (CvtDynAnyP()->currentComponentIndex() >= 0)
      {
	dynAnyP::Bval v;
	v.sv = value;
	CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_short,v);
	if (NP_nodetype() != dt_any)
	  (void) CvtDynAnyP()->nextComponent();
      }
    else
      throw CORBA::DynAny::InvalidValue();
    set_member(index,0);
  }
}

void
dynAnyUD1::insert_ushort(CORBA::UShort value)
{
  dynAnyP::Bval v;
  v.usv = value;
  int index = internal_union_value_to_index(pd_parent->tc(),v,
					    CORBA::tk_ushort);
  {
    // too bad we have to inline the body of CORBA::DynAny::insert_ushort
    // here instead of just calling it because we have to interlock
    // the insertion with set_member.
    omni_mutex_lock sync(dynAnyP::lock);
    if (CvtDynAnyP()->currentComponentIndex() >= 0)
      {
	dynAnyP::Bval v;
	v.usv = value;
	CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_ushort,v);
	if (NP_nodetype() != dt_any)
	  (void) CvtDynAnyP()->nextComponent();
      }
    else
      throw CORBA::DynAny::InvalidValue();
    set_member(index,0);
  }
}

void
dynAnyUD1::insert_long(CORBA::Long value)
{
  dynAnyP::Bval v;
  v.lv = value;
  int index = internal_union_value_to_index(pd_parent->tc(),v,
					    CORBA::tk_long);
  {
    omni_mutex_lock sync(dynAnyP::lock);
    if (CvtDynAnyP()->currentComponentIndex() >= 0)
      {
	dynAnyP::Bval v;
	v.lv = value;
	CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_long,v);
	if (NP_nodetype() != dt_any)
	  (void) CvtDynAnyP()->nextComponent();
      }
    else
      throw CORBA::DynAny::InvalidValue();
    set_member(index,0);
  }
}

void
dynAnyUD1::insert_ulong(CORBA::ULong value)
{
  dynAnyP::Bval v;
  v.ulv = value;
  int index = internal_union_value_to_index(pd_parent->tc(),v,
					    CORBA::tk_ulong);
  {
    omni_mutex_lock sync(dynAnyP::lock);
    if (CvtDynAnyP()->currentComponentIndex() >= 0)
      {
	dynAnyP::Bval v;
	v.ulv = value;
	CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_ulong,v);
	if (NP_nodetype() != dt_any)
	  (void) CvtDynAnyP()->nextComponent();
      }
    else
      throw CORBA::DynAny::InvalidValue();
    set_member(index,0);
  }
}


void
dynAnyUD1::set_member(int index,CORBA::Boolean update_value)
{
  if (index < 0) {
    if (pd_implicit_default != 1) {
      pd_implicit_default = 1;
      pd_parent->createComponent(1,1,CORBA::TypeCode::_nil());
    }
  }
  else {
    if (index != pd_member_index) {
      pd_parent->createComponent(1,1,
			   pd_parent->tc()->member_type((CORBA::ULong)index));
    }
  }
  pd_member_index = index;

  if (update_value) {
    CORBA::Any_var v;
    if (index >= 0 && index != pd_parent->tc()->default_index()) {
      CORBA::Any_var v = pd_parent->tc()->member_label((CORBA::ULong)index);
      NP_real_from_any(v);
    }
    else {
      // update member to the default value
      dynAnyP::Bval v;
      internal_union_default_value(pd_parent->tc(),v);
      ((dynAnyP*)NP_pd())->nthComponent(0);
      ((dynAnyP*)NP_pd())->
	  currentComponentFromBasetype(((dynAnyP*)NP_pd())->tc()->kind(),v);
    }
  }
}

void
dynAnyUD2::NP_real_from_any(const CORBA::Any& value)
{
  dynAnyP::Bval v;
  v.anyv = (CORBA::Any*)(&value);
  int index = internal_union_value_to_index(pd_parent->tc(),v,
					    CORBA::tk_any);
  CvtDynAnyP()->fromAny(value);
  set_member(index,0);
}

void
dynAnyUD2::value_as_string(const char* value)
{
  CORBA::TypeCode_ptr tc = ((dynAnyP*)NP_pd())->tc();
  CORBA::ULong numMems = tc->member_count();
  for(CORBA::ULong count = 0; count < numMems; count++)
    {
      if (strcmp(value,tc->member_name(count)) == 0)
	{
	  value_as_ulong(count);
	  return;
	}
    }
  throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
}

void
dynAnyUD2::value_as_ulong(CORBA::ULong value)
{
  omni_mutex_lock sync(dynAnyP::lock);
  dynAnyP::Bval v;
  v.ulv = value;
  int index = internal_union_value_to_index(pd_parent->tc(),v,
					    CORBA::tk_enum);
  ((dynAnyP*)NP_pd())->currentComponentFromBasetype(CORBA::tk_enum,v);
  set_member(index,0);
}

void
dynAnyUD2::set_member(int index,CORBA::Boolean update_value)
{
  if (index < 0) {
    if (pd_implicit_default != 1) {
      pd_implicit_default = 1;
      pd_parent->createComponent(1,1,CORBA::TypeCode::_nil());
    }
  }
  else {
    if (index != pd_member_index) {
      pd_parent->createComponent(1,1,
		      pd_parent->tc()->member_type((CORBA::ULong)index));
    }
  }
  pd_member_index = index;

  if (update_value) {
    CORBA::Any_var v;
    if (index >= 0 && index != pd_parent->tc()->default_index()) {
      v = pd_parent->tc()->member_label((CORBA::ULong)index);
      NP_real_from_any(v);
    }
    else {
      // update member to the default value
      dynAnyP::Bval v;
      internal_union_default_value(pd_parent->tc(),v);
      ((dynAnyP*)NP_pd())->nthComponent(0);
      ((dynAnyP*)NP_pd())->
	  currentComponentFromBasetype(((dynAnyP*)NP_pd())->tc()->kind(),v);
    }
  }
}

CORBA::Boolean
CORBA::
DynUnion::set_as_default()
{
  omni_mutex_lock sync(dynAnyP::lock);

  CORBA::TypeCode_ptr t = CvtDynAnyP()->tc();
  int dpos = t->default_index();

  CvtDynAnyP()->nthComponent(0);
  DynAny_var v = CvtDynAnyP()->currentComponent();
  int mpos;
  if (v->NP_nodetype() == dt_any) {
    // Discriminator is a scaler.
    // The dynAny *MUST BE* dynAnyUD1.
    // Make sure that this is in sync with the dynAnyP implementation for
    // union.
    dynAnyUD1* p = (dynAnyUD1*) ((CORBA::DynAny_ptr)v);
    mpos = p->get_member();
    if (dpos < 0 && mpos < 0) {
      return p->implicit_default_member();
    }
  }
  else {
    // Discriminator is a enum.
    // The dynAny *MUST BE* dynAnyUD2.
    // Make sure that this is in sync with the dynAnyP implementation for
    // union.
    dynAnyUD2* p = (dynAnyUD2*) ((CORBA::DynAny_ptr)v);
    mpos = p->get_member();
    if (dpos < 0 && mpos < 0) {
      return p->implicit_default_member();
    }
  }
  if (mpos < 0 || mpos != dpos)
    return 0;
  else
    return 1;
}


void
CORBA::
DynUnion::set_as_default(CORBA::Boolean value)
{
  omni_mutex_lock sync(dynAnyP::lock);

  CORBA::TypeCode_ptr t = CvtDynAnyP()->tc();
  int dpos = t->default_index();
  if (dpos < 0) {
    // No explicit default member, check if this union has an implicit
    // default member, i.e. if it does not have a default case and not all
    // permissible values of the union discriminant are listed.
    if (internal_union_no_default_value(t))
	return;
  }

  CvtDynAnyP()->nthComponent(0);
  DynAny_var v = CvtDynAnyP()->currentComponent();
  int mpos;
  if (v->NP_nodetype() == dt_any) {
    // Discriminator is a scaler.
    // The dynAny *MUST BE* dynAnyUD1.
    // Make sure that this is in sync with the dynAnyP implementation for
    // union.
    dynAnyUD1* p = (dynAnyUD1*) ((CORBA::DynAny_ptr)v);
    mpos = p->get_member();
    if (value) {
      // Set to default member if it is not already
      if (dpos >= 0) {
	if (mpos != dpos)
	  p->set_member(dpos);
      }
      else if (!p->implicit_default_member()) {
	p->set_member(-1);
      }
    }
    else {
      // Do not select the default member? What does this mean?
      // XXX Ignore this op.
    }
  }
  else {
    // Discriminator is a enum.
    // The dynAny *MUST BE* dynAnyUD2.
    // Make sure that this is in sync with the dynAnyP implementation for
    // union.
    dynAnyUD2* p = (dynAnyUD2*) ((CORBA::DynAny_ptr)v);
    mpos = p->get_member();
    if (value) {
      // Set to default member if it is not already
      if (dpos >= 0) {
	if (mpos != dpos)
	  p->set_member(dpos);
      }
      else if (!p->implicit_default_member()) {
	p->set_member(-1);
      }
    }
    else {
      // Do not select the default member? What does this mean?
      // XXX Ignore this op.
    }
  }
}

// internal_initialise_buffer
// internal_rewind_buffer
//
// This implementation used CORBA::Any::NP_getBuffer and
// CORBA::Any::NP_replaceData to obtain and replace the data of an
// any. The private data member pd_buf is used as the argument to
// these functions. The buffer must satisfy the internal invariant as
// specified in the Any class (see CORBA.h). The two helper functions
// internal_initialise_buffer and internal_rewind_buffer ensures that
// the buffer is correctly setup.
//
static
void
internal_initialise_buffer(MemBufferedStream& buf,CORBA::TypeCode_ptr tc)
{
  buf.rewind_inout_mkr();
  CORBA::ULong padding = tc->NP_alignedSize(0) % 4;
  while (padding-- > 0) {
    CORBA::Octet c = 0;
    c >>= buf;
  }
}

static
void
internal_rewind_buffer(MemBufferedStream& buf,CORBA::TypeCode_ptr tc)
{
  buf.rewind_in_mkr();
  CORBA::ULong padding = tc->NP_alignedSize(0) % 4;
  if (padding) {
    buf.skip(padding);
  }
  if (tc->kind() == CORBA::tk_sequence) {
    // skip the length field in the buffer stream.
    buf.skip(4,omni::ALIGN_4);
  }
}

static
CORBA::ULong
internal_buffer_getpos(MemBufferedStream& buf)
{
  return buf.RdMessageAlreadyRead();
}

static
void
internal_buffer_setpos(MemBufferedStream& buf,CORBA::ULong pos)
{
  buf.rewind_in_mkr();
  buf.skip(pos);
}


static
CORBA::ULong
internal_read_seqlen_from_buffer(MemBufferedStream& buf,
				 CORBA::TypeCode_ptr tc)
{
  try {
    CORBA::ULong savepos = internal_buffer_getpos(buf);
    buf.rewind_in_mkr();
    CORBA::ULong padding = tc->NP_alignedSize(0) % 4;
    if (padding) {
      buf.skip(padding);
    }
    CORBA::ULong size;
    size <<= buf;
    internal_buffer_setpos(buf,savepos);
    return size;
  }
  catch(...) {
    throw;
    return 0;
  }
}


static
void
internal_write_seqlen_into_buffer(MemBufferedStream& buf,
				  CORBA::TypeCode_ptr tc,
				  CORBA::ULong len)
{
  try {
    size_t savepos = internal_buffer_getpos(buf);
    buf.rewind_in_mkr();
    CORBA::ULong padding = tc->NP_alignedSize(0) % 4;
    if (padding) {
      buf.skip(padding);
    }
    if (buf.WrMessageAlreadyWritten() == buf.RdMessageAlreadyRead()) {
      // the length value has not been written
      len >>= buf;
    }
    else {
      // check if only the length value and nothing else has been written
      buf.skip(0,omni::ALIGN_4);
      if (buf.WrMessageAlreadyWritten() <= buf.RdMessageAlreadyRead() + 4) {
	internal_initialise_buffer(buf,tc);
	len >>= buf;
      }
      else {
	// Some elements of the sequence have already be marshalled into
	// the buffer. We now want to change the length value encoded at
	// the beginning of the stream.
	// We could do this correctly but inefficently by marshalling the
	// new length value and the sequence content into a temporary
	// MemBufferedStream and then use that to overwrite buf.
	// XXX - Alternatively, and this is a (bad) hack, we change the
	//       length in situ. To do so, we use MemBufferedStream::data()
	//       to obtain a pointer into the buffer where the length
	//       value is encoded.
	CORBA::ULong* p = (CORBA::ULong*)buf.data();
	// Make sure that we put in the value in the same endian as
	// the rest of the content in the buffer
	//
	if (buf.byteOrder() == omni::myByteOrder) {
	  *p = len;
	}
	else {
	  *p = ((((len) & 0xff000000) >> 24) |
		(((len) & 0x00ff0000) >> 8)  |
		(((len) & 0x0000ff00) << 8)  |
		(((len) & 0x000000ff) << 24));
	}
      }
    }
    internal_buffer_setpos(buf,savepos);
  }
  catch (...) { 
    throw;
  }
}

// internal_union_value_to_index
//   Given the typecode of the union <tc>, and a value <v> of its
//   discriminator, returns the index of the member in the typecode that
//   correspond to the value of the discriminator.
//   If the value match none of the labelled member (including default),
//   returns -1.
//
//   The discriminator value is passed as a Bval union, its instantiated
//   branch is identified by the kind field <k>. The valid kinds are:
//      short,long,ushort,ulong,boolean,char,octet,enum,any.
//   If the kind is any, the content of the any is extracted to obtain
//   the value of the discriminator.
//   If there is a mismatch in type or value, a InvalidValue exception
//   is raised.
static
int
internal_union_value_to_index(CORBA::TypeCode_ptr tc, dynAnyP::Bval v,
			      CORBA::TCKind k)
{
  CORBA::TypeCode_var disctype = tc->discriminator_type();

  if (k == CORBA::tk_any) {
    // If the value is an any, we have to extract its value here.
    CORBA::Any* anyv = v.anyv;
    CORBA::TypeCode_var anytc = anyv->type();

    if (!disctype->NP_expandEqual(anytc,1))
      throw CORBA::DynAny::InvalidValue();

    switch (anytc->kind()) {

    case CORBA::tk_short:
      {
	*((CORBA::Any*)anyv) >>= v.sv;
	k = CORBA::tk_short;
	break;
      }
    case CORBA::tk_long:
      {
	*((CORBA::Any*)anyv) >>= v.lv;
	k = CORBA::tk_long;
	break;
      }
    case CORBA::tk_ushort:
      {
	*((CORBA::Any*)anyv) >>= v.usv;
	k = CORBA::tk_ushort;
	break;
      }
    case CORBA::tk_ulong:
      {
	*((CORBA::Any*)anyv) >>= v.ulv;
	k = CORBA::tk_ulong;
	break;
      }
    case CORBA::tk_boolean:
      {
	*((CORBA::Any*)anyv) >>= CORBA::Any::to_boolean(v.bv);
	k = CORBA::tk_boolean;
	break;
      }
    case CORBA::tk_char:
      {
	*((CORBA::Any*)anyv) >>= CORBA::Any::to_char(v.cv);
	k = CORBA::tk_char;
	break;
      }
    case CORBA::tk_octet:
      {
	*((CORBA::Any*)anyv) >>= CORBA::Any::to_octet(v.ov);
	k = CORBA::tk_octet;
	break;
      }
    case CORBA::tk_enum:
      {
	// Extract the enum value as a ulong.
	MemBufferedStream buf;
	anyv->NP_getBuffer(buf);
	v.ulv <<= buf;
	k = CORBA::tk_enum;
	break;
      }
    default:
      throw CORBA::DynAny::InvalidValue();
    }
  }


  if (disctype->kind() != k) {
    throw CORBA::DynAny::InvalidValue();
  }


  CORBA::Long defaultindex = tc->default_index();
  CORBA::Long total = (CORBA::Long)tc->member_count();
  for (CORBA::Long index = 0; index < total; index++)
    {
      if (index == defaultindex) continue;
      CORBA::Any_var value = tc->member_label(index);
      switch (k) {
      case CORBA::tk_short:
	{
	  CORBA::Short u;
	  value >>= u;
	  if (u == v.sv)
	    return index;
	  break;
	}
      case CORBA::tk_long:
	{
	  CORBA::Long u;
	  value >>= u;
	  if (u == v.lv)
	    return index;
	  break;
	}
      case CORBA::tk_ushort:
	{
	  CORBA::UShort u;
	  value >>= u;
	  if (u == v.usv)
	    return index;
	  break;
	}
      case CORBA::tk_ulong:
	{
	  CORBA::ULong u;
	  value >>= u;
	  if (u == v.ulv)
	    return index;
	  break;
	}
      case CORBA::tk_boolean:
	{
	  CORBA::Boolean u;
	  value >>= CORBA::Any::to_boolean(u);
	  if (u == v.bv)
	    return index;
	  break;
	}
      case CORBA::tk_char:
	{
	  CORBA::Char u;
	  value >>= CORBA::Any::to_char(u);
	  if (u == v.cv)
	    return index;
	  break;
	}
      case CORBA::tk_octet:
	{
	  CORBA::Octet u;
	  value >>= CORBA::Any::to_octet(u);
	  if (u == v.ov)
	    return index;
	  break;
	}
      case CORBA::tk_enum:
	{
	  // Extract the enum value as a ulong.
	  CORBA::ULong u;
	  MemBufferedStream buf;
	  value->NP_getBuffer(buf);
	  u <<= buf;
	  if (u == v.ulv)
	    return index;
	  break;
	}
      default:
	throw CORBA::DynAny::InvalidValue();
      }
  }
  // Reach here if the value match none of the labels.
  // If there is a default branch, returns its index, otherwise returns -1.
  return defaultindex;
}

// internal_union_default_value
//   Given the typecode of the union <tc>, produce a value that can
//   be used as the default value of the discriminator.
//   Caller of this function must ensure that the union either has a
//   default branch or there are some values in the discriminator's valid
//   range that have not been used as branch label.
static
void
internal_union_default_value(CORBA::TypeCode_ptr tc, dynAnyP::Bval& retval)
{
  CORBA::TypeCode_var dtc = tc->discriminator_type();
  CORBA::Long defaultindex = tc->default_index();

  switch (dtc->kind()) {

  case CORBA::tk_short:
    {
      retval.sv = -((CORBA::Short)0x7fff);
      while (internal_union_value_to_index(tc,retval,CORBA::tk_short)
	     != defaultindex) {
	retval.sv++;
      }
      break;
    }
  case CORBA::tk_long:
    {
      retval.lv = -((CORBA::Long)0x7fffffff);
      while (internal_union_value_to_index(tc,retval,CORBA::tk_long)
	     != defaultindex) {
	retval.lv++;
      }
      break;
    }
  case CORBA::tk_ushort:
    {
      retval.usv = 0;
      while (internal_union_value_to_index(tc,retval,CORBA::tk_ushort)
	     != defaultindex) {
	retval.usv++;
      }
      break;
    }
  case CORBA::tk_ulong:
    {
      retval.ulv = 0;
      while (internal_union_value_to_index(tc,retval,CORBA::tk_ulong)
	     != defaultindex) {
	retval.ulv++;
      }
      break;
    }
  case CORBA::tk_boolean:
    {
      retval.bv = 0;
      if (internal_union_value_to_index(tc,retval,CORBA::tk_boolean) !=
	  defaultindex)
	{
	  retval.bv = 1;
	}
      break;
    }
  case CORBA::tk_char:
    {
      retval.cv = 0;
      while (internal_union_value_to_index(tc,retval,CORBA::tk_char)
	     != defaultindex) {
	retval.cv++;
      }
      break;
    }
  case CORBA::tk_octet:
    {
      retval.ov = 0;
      while (internal_union_value_to_index(tc,retval,CORBA::tk_octet)
	     != defaultindex) {
	retval.ov++;
      }
      break;
    }
  case CORBA::tk_enum:
    {
      CORBA::ULong total = dtc->member_count();
      for (CORBA::ULong index = 0; index < total; index++) {
	retval.ulv = index;
	if (internal_union_value_to_index(tc,retval,CORBA::tk_enum)
	    == defaultindex) {
	  break;
	}
      }
      break;
    }
  default:
    throw CORBA::DynAny::InvalidValue();
  }
}

// internal_union_no_default_value
//    Given the typecode of the union <tc>, returns 1 if all the possible
//    values of the discriminant are used as union branch labels. returns 0
//    otherwise.

static
CORBA::Boolean
internal_union_no_default_value(CORBA::TypeCode_ptr tc)
{
  if (tc->default_index() >= 0) return 0;

  CORBA::TypeCode_var dtc = tc->discriminator_type();
  CORBA::ULong npossible = 0;

  switch (dtc->kind()) {

  case CORBA::tk_short:
  case CORBA::tk_ushort:
    npossible = 65536;
    break;
  case CORBA::tk_long:
  case CORBA::tk_ulong:
    npossible = 0xffffffff;
    break;
  case CORBA::tk_boolean:
    npossible = 2;
    break;
  case CORBA::tk_char:
  case CORBA::tk_octet:
    npossible = 256;
    break;
  case CORBA::tk_enum:
    npossible = dtc->member_count();
    break;
  default:
    throw CORBA::DynAny::InvalidValue();
  }
  if (npossible > tc->member_count())
    return 0;
  else
    return 1;
}
