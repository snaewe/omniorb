// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcParser.cc                Created on: 8/1998
//                            Author1   : James Weatherall (jnw)
//                            Author2   : David Riddoch (djr)
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

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <tcParser.h>
#include <typecode.h>
#include <codeSetUtil.h>
#include <omniORB4/valueType.h>


OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
/////////////////////// Internal Implementation //////////////////////
//////////////////////////////////////////////////////////////////////

//?? Further optimisations possible if source and destination have
// the same alignment at any point - 'cos we can then just byte
// copy EVERYTHING from there onwards.
// ie. if( src_posn % 8 == dst_posn % 8 ) ...
//  The only interesting question is knowing when to stop.
//  Code set conversion means that we would have to stop at any char
//  or wchar data.

inline void fastCopyUsingTC(TypeCode_base* tc, cdrStream& ibuf, cdrStream& obuf)
{
  // This can only be used if both streams have the same
  // byte order.

  // Follow any TypeCode_indirect objects created by
  // ORB::create_recursive_tc().
  tc = TypeCode_indirect::strip(tc);

  const TypeCode_alignTable& alignTbl = tc->alignmentTable();
  unsigned i = 0;  // don't even ask ... just accept it.

  OMNIORB_ASSERT(alignTbl.entries() > 0);

  for( i = 0; i < alignTbl.entries(); i++ ) {

    switch( alignTbl[i].type ) {
    case TypeCode_alignTable::it_simple:
      // Can copy data across as a block.
      ibuf.copy_to(obuf,alignTbl[i].simple.size,
		   alignTbl[i].simple.alignment);
      break;

    case TypeCode_alignTable::it_nasty:
      { // Data is variable length or has variable alignment requirements.
	tc = alignTbl[i].nasty.tc;

	switch( tc->NP_kind() ) {

	  //?? Some of these could be faster (Any, TypeCode, objref ...)

	case CORBA::tk_char:
	  {
	    if (ibuf.TCS_C() == obuf.TCS_C()) {
	      // No conversion necessary
	      CORBA::Octet o = ibuf.unmarshalOctet(); obuf.marshalOctet(o);
	    }
	    else {
	      omniCodeSet::UniChar uc = ibuf.TCS_C()->unmarshalChar(ibuf);
	      obuf.TCS_C()->marshalChar(obuf, uc);
	    }
	    break;
	  }

	case CORBA::tk_wchar:
	  {
	    if (ibuf.TCS_W() == obuf.TCS_W()) {
	      // No conversion necessary
	      CORBA::Octet len = ibuf.unmarshalOctet();
	      obuf.marshalOctet(len);
	      ibuf.copy_to(obuf, len, omni::ALIGN_1);
	    }
	    else {
	      omniCodeSet::UniChar uc = ibuf.TCS_W()->unmarshalWChar(ibuf);
	      obuf.TCS_W()->marshalWChar(obuf, uc);
	    }
	    break;
	  }

	case CORBA::tk_any:
	  { CORBA::Any d; d <<= ibuf; d >>= obuf; break; }

	case CORBA::tk_Principal:
	  {
	    CORBA::ULong len;
	    len <<= ibuf;
	    len >>= obuf;
	    ibuf.copy_to(obuf,len);
	    break;
	  }

	case CORBA::tk_objref:
	  { CORBA::Object_Member d; d <<= ibuf; d >>= obuf; break; }

	case CORBA::tk_TypeCode:
	  { CORBA::TypeCode_member d; d <<= ibuf; d >>= obuf; break; }

	case CORBA::tk_string:
	  {
	    if (ibuf.TCS_C() == obuf.TCS_C()) {
	      // No conversion necessary
	      CORBA::ULong len;
	      len <<= ibuf;
	      len >>= obuf;
	      ibuf.copy_to(obuf,len);
	    }
	    else {
	      // Convert via UTF-16
	      omniCodeSet::UniChar* us;
	      CORBA::ULong len = ibuf.TCS_C()->unmarshalString(ibuf, 0, us);
	      omniCodeSetUtil::HolderU uh(us);
	      obuf.TCS_C()->marshalString(obuf, len, us);
	    }
	    break;
	  }

	case CORBA::tk_wstring:
	  {
	    if (ibuf.TCS_W() == obuf.TCS_W()) {
	      // No conversion necessary
	      CORBA::ULong len;
	      len <<= ibuf;
	      len >>= obuf;
	      ibuf.copy_to(obuf,len);
	    }
	    else {
	      // Convert via UTF-16
	      omniCodeSet::UniChar* us;
	      CORBA::ULong len = ibuf.TCS_W()->unmarshalWString(ibuf, 0, us);
	      omniCodeSetUtil::HolderU uh(us);
	      obuf.TCS_W()->marshalWString(obuf, len, us);
	    }
	    break;
	  }

	case CORBA::tk_union:
	  {
	    // Fetch and copy the discriminator value.
	    TypeCode_base* discrimTC = tc->NP_discriminator_type();
	    TypeCode_union::Discriminator discrim =
	      TypeCode_union_helper::unmarshalLabel(discrimTC, ibuf);
	    TypeCode_union_helper::marshalLabel(discrim, discrimTC, obuf);

	    // Copy the value, using the type for the selected member,
	    // unless we have an implicit default, in which case no
	    // value is copied.
	    CORBA::Long i =
	      ((TypeCode_union*)tc)->NP_index_from_discriminator(discrim);
	    if( i >= 0 )
	      fastCopyUsingTC(tc->NP_member_type(i), ibuf, obuf);
	    break;
	  }

	case CORBA::tk_sequence:
	  {
	    CORBA::ULong length; length <<= ibuf; length >>= obuf;
	    if( !length )  break;

	    TypeCode_base* elem_tc = tc->NP_content_type();
	    elem_tc = TypeCode_indirect::strip(elem_tc);
	    const TypeCode_alignTable& eat = elem_tc->alignmentTable();

	    if( eat.is_simple() ) {
	      // Copy as a single block.
	      CORBA::ULong size_aligned =
		omni::align_to(eat[0].simple.size, eat[0].simple.alignment);
	      ibuf.copy_to(obuf, eat[0].simple.size +
			     (length - 1) * size_aligned,
			     eat[0].simple.alignment);
	    }
	    else if( eat.has_only_simple() ) {
	      // Copy the first element ...
	      CORBA::ULong start = 0;
	      for( unsigned j = 0; j < eat.entries(); j++ ) {
		start = omni::align_to(start, eat[j].simple.alignment);
		start += eat[j].simple.size;
		ibuf.copy_to(obuf, eat[j].simple.size,
			     eat[j].simple.alignment);
	      }
	      // Calculate the size of subsequent elements ...
	      CORBA::ULong end = start;
	      for( unsigned k = 0; k < eat.entries(); k++ ) {
		end = omni::align_to(end, eat[k].simple.alignment);
		end += eat[k].simple.size;
	      }
	      // ... then copy the rest as a block.
	      ibuf.copy_to(obuf, (length - 1) * (end - start));
	    }
	    else {
	      // We can't do better than copying element by element.
	      for( CORBA::ULong j = 0 ; j < length; j++ )
		fastCopyUsingTC(elem_tc, ibuf, obuf);
	    }

	    break;
	  }

	case CORBA::tk_array:
	  {
	    // If element type is_simple(), or has_only_simple()
	    // then it will have been dealt with above. We only
	    // get here if the best we can do is to copy element
	    // by element.

	    CORBA::ULong length = tc->NP_length();
	    TypeCode_base* elemTC = tc->NP_content_type();

	    for( CORBA::ULong j = 0; j < length; j++ )
	      fastCopyUsingTC(elemTC, ibuf, obuf);

	    break;
	  }

	case CORBA::tk_alias:
	  fastCopyUsingTC(tc->NP_content_type(), ibuf, obuf);
	  break;

	case CORBA::tk_fixed:
	  {
	    CORBA::Fixed f;
	    f.PR_setLimits(tc->NP_fixed_digits(), tc->NP_fixed_scale());
	    f <<= ibuf;
	    f >>= obuf;
	    break;
	  }
	  
	case CORBA::tk_value:
	case CORBA::tk_value_box:
	  {
	    CORBA::ULong hash = omniValueType::hash_id(tc->id());
	    CORBA::ValueBase* v = omniValueType::unmarshal(tc->id(), hash,
							   tc, ibuf);
	    omniValueType::marshal(v, tc->id(), obuf);
	    CORBA::remove_ref(v);
	  }

	default:
	  OMNIORB_ASSERT(0);
	}
	break;
      }

    default:
      OMNIORB_ASSERT(0);
    }
  }
}

void copyUsingTC(TypeCode_base* tc, cdrStream& ibuf, cdrStream& obuf)
{
  tc = TypeCode_indirect::strip(tc);

  // How to marshal the data depends entirely on the TypeCode
  switch (tc->NP_kind())
    {

      // TYPES WITH NO DATA TO MARSHAL
    case CORBA::tk_null:
    case CORBA::tk_void:
      return;

      // SIMPLE TYPES
    case CORBA::tk_short:
      { CORBA::Short d;   d <<= ibuf; d >>= obuf; return; }
    case CORBA::tk_ushort:
      { CORBA::UShort d;  d <<= ibuf; d >>= obuf; return; }
    case CORBA::tk_long:
      { CORBA::Long d;    d <<= ibuf; d >>= obuf; return; }
    case CORBA::tk_ulong:
      { CORBA::ULong d;   d <<= ibuf; d >>= obuf; return; }
    case CORBA::tk_float:
      { CORBA::Float d;   d <<= ibuf; d >>= obuf; return; }
    case CORBA::tk_double:
      { CORBA::Double d;  d <<= ibuf; d >>= obuf; return; }
    case CORBA::tk_boolean:
      { CORBA::Boolean d; d = ibuf.unmarshalBoolean(); 
        obuf.marshalBoolean(d); return; }
    case CORBA::tk_octet:
      { CORBA::Octet d; d = ibuf.unmarshalOctet();
      obuf.marshalOctet(d); return; }
    case CORBA::tk_enum:
      { CORBA::ULong d;   d <<= ibuf; d >>= obuf; return; }

#ifdef HAS_LongLong
    case CORBA::tk_longlong:
      { CORBA::LongLong d;    d <<= ibuf; d >>= obuf; return; }
    case CORBA::tk_ulonglong:
      { CORBA::ULongLong d;   d <<= ibuf; d >>= obuf; return; }
#endif
#ifdef HAS_LongDouble
    case CORBA::tk_longdouble:
      { CORBA::LongDouble d;  d <<= ibuf; d >>= obuf; return; }
#endif

    case CORBA::tk_any:
      { CORBA::Any d;     d <<= ibuf; d >>= obuf; return; }

    // COMPLEX TYPES
    case CORBA::tk_char:
      {
	if (ibuf.TCS_C() == obuf.TCS_C()) {
	  // No conversion necessary
	  CORBA::Octet o = ibuf.unmarshalOctet(); obuf.marshalOctet(o);
	}
	else {
	  omniCodeSet::UniChar uc = ibuf.TCS_C()->unmarshalChar(ibuf);
	  obuf.TCS_C()->marshalChar(obuf, uc);
	}
	return;
      }

    case CORBA::tk_wchar:
      {
	// Always do a conversion via UTF-16, to avoid the pain of
	// possibly byteswapping.
	omniCodeSet::UniChar uc = ibuf.TCS_W()->unmarshalWChar(ibuf);
	obuf.TCS_W()->marshalWChar(obuf, uc);
	return;
      }

    case CORBA::tk_Principal:
      {
	CORBA::ULong len;
	len <<= ibuf;
	len >>= obuf;
	ibuf.copy_to(obuf,len);
	return;
      }

    case CORBA::tk_objref:
      { CORBA::Object_Member d; d <<= ibuf; d >>= obuf; return; }

    case CORBA::tk_TypeCode:
      { CORBA::TypeCode_member d; d <<= ibuf; d >>= obuf; return; }

    case CORBA::tk_string:
      {
	// Convert via UTF-16
	omniCodeSet::UniChar* us;
	CORBA::ULong len = ibuf.TCS_C()->unmarshalString(ibuf, 0, us);
	omniCodeSetUtil::HolderU uh(us);
	obuf.TCS_C()->marshalString(obuf, len, us);
	return;
      }
      

    case CORBA::tk_wstring:
      {
	// Convert via UTF-16
	omniCodeSet::UniChar* us;
	CORBA::ULong len = ibuf.TCS_W()->unmarshalWString(ibuf, 0, us);
	omniCodeSetUtil::HolderU uh(us);
	obuf.TCS_W()->marshalWString(obuf, len, us);
	return;
      }

    // CONSTRUCTED TYPES
    case CORBA::tk_union:
      {
	// Fetch and copy the discriminator value
	TypeCode_base* discrimTC = tc->NP_discriminator_type();
	TypeCode_union::Discriminator discrim =
	  TypeCode_union_helper::unmarshalLabel(discrimTC, ibuf);
	TypeCode_union_helper::marshalLabel(discrim, discrimTC, obuf);

	// Copy the value, using the type for the selected member,
	// unless we have an implicit default, in which case no
	// value is copied.
	CORBA::Long i =
	  ((TypeCode_union*)tc)->NP_index_from_discriminator(discrim);
	if( i >= 0 )
	  copyUsingTC(tc->NP_member_type(i), ibuf, obuf);

	return;
      }

    case CORBA::tk_struct:
      {
	CORBA::ULong nmembers = tc->NP_member_count();

	// Copy the individual elements.
	for (CORBA::ULong i=0; i < nmembers; i++)
	  copyUsingTC(tc->NP_member_type(i), ibuf, obuf);

	return;
      }
    
    case CORBA::tk_except:
      {
	// Exceptions are passed on the wire as repo id followed
	// by members - but for contents of Any we are only interested
	// in members. Therefore we copy the members only here, and
	// the stubs/GIOP_S code deals with the repo id.

	CORBA::ULong nmembers = tc->NP_member_count();

	// Copy the individual elements.
	for (CORBA::ULong i=0; i < nmembers; i++)
	  copyUsingTC(tc->NP_member_type(i), ibuf, obuf);

	return;
      }

    case CORBA::tk_sequence:
      {
	CORBA::ULong max; max <<= ibuf; max >>= obuf;
	TypeCode_base* tctmp = tc->NP_content_type();

	for (CORBA::ULong i=0; i < max; i++)
	  copyUsingTC(tctmp, ibuf, obuf);

	return;
      }

    case CORBA::tk_array:
      {
	CORBA::ULong max = tc->NP_length();
	TypeCode_base* tctmp = tc->NP_content_type();

	for (CORBA::ULong i=0; i < max; i++)
	  copyUsingTC(tctmp, ibuf, obuf);

	return;
      }

    case CORBA::tk_alias:
      {
	// This could be made more efficient
	copyUsingTC(tc->NP_content_type(), ibuf, obuf);
	return;
      }

    case CORBA::tk_fixed:
      {
	CORBA::Fixed f;
	f.PR_setLimits(tc->NP_fixed_digits(), tc->NP_fixed_scale());
	f <<= ibuf;
	f >>= obuf;
	break;
      }

    case CORBA::tk_value:
    case CORBA::tk_value_box:
      {
	CORBA::ULong hash = omniValueType::hash_id(tc->id());
	CORBA::ValueBase* v = omniValueType::unmarshal(tc->id(), hash,
						       tc, ibuf);
	omniValueType::marshal(v, tc->id(), obuf);
	CORBA::remove_ref(v);
      }

    default:
      OMNIORB_ASSERT(0);
    }
}


void skipUsingTC(TypeCode_base* tc, cdrStream& buf)
{
  tc = TypeCode_indirect::strip(tc);

  CORBA::Char dummy;
  const TypeCode_alignTable& alignTbl = tc->alignmentTable();
  unsigned i = 0;  // don't even ask ... just accept it.

  OMNIORB_ASSERT(alignTbl.entries() > 0);

  for( i = 0; i < alignTbl.entries(); i++ ) {

    switch( alignTbl[i].type ) {
    case TypeCode_alignTable::it_simple:
      buf.alignInput(alignTbl[i].simple.alignment);
      buf.skipInput(alignTbl[i].simple.size);
      break;

    case TypeCode_alignTable::it_nasty:
      {
	tc = alignTbl[i].nasty.tc;

	switch( tc->NP_kind() ) {

	  //?? Some of these could be faster (Any, TypeCode, objref ...)
	case CORBA::tk_char:
	  { buf.unmarshalOctet(); break; }

	case CORBA::tk_wchar:
	  {
	    CORBA::Octet len = buf.unmarshalOctet();
	    buf.skipInput(len);
	    break;
	  }

	case CORBA::tk_any:
	  { CORBA::Any d; d <<= buf; break; }

	case CORBA::tk_Principal:
	case CORBA::tk_string:
	  {
	    CORBA::ULong len;
	    len <<= buf;
	    buf.skipInput(len);
	    break;
	  }

	case CORBA::tk_wstring:
	  {
	    CORBA::ULong len;
	    len <<= buf;
	    buf.skipInput(len);
	    break;
	  }

	case CORBA::tk_objref:
	  { CORBA::Object_Member d; d <<= buf; break; }

	case CORBA::tk_TypeCode:
	  { CORBA::TypeCode_member d; d <<= buf; break; }

	case CORBA::tk_union:
	  {
	    // Fetch the discriminator value.
	    TypeCode_base* discrimTC = tc->NP_discriminator_type();
	    TypeCode_union::Discriminator discrim =
	      TypeCode_union_helper::unmarshalLabel(discrimTC, buf);

	    // Skip the value, using the type for the selected member.
	    CORBA::Long i =
	      ((TypeCode_union*)tc)->NP_index_from_discriminator(discrim);
	    if( i >= 0 )  skipUsingTC(tc->NP_member_type(i), buf);
	    break;
	  }

	case CORBA::tk_sequence:
	  {
	    CORBA::ULong length; length <<= buf;
	    if( !length )  break;

	    TypeCode_base* elem_tc = tc->NP_content_type();
	    elem_tc = TypeCode_indirect::strip(elem_tc);
	    const TypeCode_alignTable& eat = elem_tc->alignmentTable();

	    if( eat.is_simple() ) {
	      // Skip as a single block.
	      CORBA::ULong size_aligned =
		omni::align_to(eat[0].simple.size, eat[0].simple.alignment);
	      buf.alignInput(eat[0].simple.alignment);
	      buf.skipInput(eat[0].simple.size + (length - 1) * size_aligned);
	    }
	    else if( eat.has_only_simple() ) {
	      // Skip the first element ...
	      CORBA::ULong start = 0;
	      for( unsigned j = 0; j < eat.entries(); j++ ) {
		start = omni::align_to(start, eat[j].simple.alignment);
		start += eat[j].simple.size;
		buf.alignInput(eat[j].simple.alignment);
		buf.skipInput(eat[j].simple.size);
	      }
	      // Calculate the size of subsequent elements ...
	      CORBA::ULong end = start;
	      for( unsigned k = 0; k < eat.entries(); k++ ) {
		end = omni::align_to(end, eat[k].simple.alignment);
		end += eat[k].simple.size;
	      }
	      // ... then skip the rest as a block.
	      buf.skipInput((length - 1) * (end - start));
	    }
	    else {
	      // We can't do better than skipping element by element.
	      for( CORBA::ULong j = 0; j < length; j++ )
		skipUsingTC(elem_tc, buf);
	    }

	    break;
	  }

	case CORBA::tk_array:
	  {
	    // If element type is_simple(), or has_only_simple()
	    // then it will have been deal with above. We only
	    // get here if the best we can do is to copy element
	    // by element.

	    CORBA::ULong length = tc->NP_length();
	    TypeCode_base* elemTC = tc->NP_content_type();

	    for( CORBA::ULong j = 0; j < length; j++ )
	      skipUsingTC(elemTC, buf);

	    break;
	  }

	case CORBA::tk_alias:
	  skipUsingTC(tc->NP_content_type(), buf);
	  break;

	case CORBA::tk_fixed:
	  {
	    CORBA::Fixed f;
	    f.PR_setLimits(tc->NP_fixed_digits(), tc->NP_fixed_scale());
	    f <<= buf;
	    break;
	  }

	case CORBA::tk_value:
	case CORBA::tk_value_box:
	  {
	    CORBA::ULong hash = omniValueType::hash_id(tc->id());
	    CORBA::ValueBase* v = omniValueType::unmarshal(tc->id(), hash,
							   tc, buf);
	    CORBA::remove_ref(v);
	  }

	default:
	  OMNIORB_ASSERT(0);
	}
	break;
      }

    default:
      OMNIORB_ASSERT(0);
    }
  }
}

//////////////////////////////////////////////////////////////////////
////////////////////////////// tcParser //////////////////////////////
//////////////////////////////////////////////////////////////////////

void
tcParser::copyStreamToStream(const CORBA::TypeCode_ptr tc,
			     cdrStream& src,
			     cdrStream& dest) {
  if (src.unmarshal_byte_swap() == dest.marshal_byte_swap())
    fastCopyUsingTC(ToTcBase_Checked(tc), src, dest);
  else
    copyUsingTC(ToTcBase_Checked(tc), src, dest);
}

void
tcParser::skip(const CORBA::TypeCode_ptr tc, cdrStream &s) {
  skipUsingTC(ToTcBase_Checked(tc), s);
}


OMNI_NAMESPACE_END(omni)

