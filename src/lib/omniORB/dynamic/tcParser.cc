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
  // Follow any TypeCode_indirect objects created by
  // ORB::create_recursive_tc().
  while (tc->NP_kind() == 0xffffffff) {
    tc = ((TypeCode_indirect*)tc)->NP_resolved();
  }

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

    default:
      OMNIORB_ASSERT(0);
    }
}


void skipUsingTC(TypeCode_base* tc, cdrStream& buf)
{
  // Follow any TypeCode_indirect objects created by
  // ORB::create_recursive_tc().
  while (tc->NP_kind() == 0xffffffff) {
    tc = ((TypeCode_indirect*)tc)->NP_resolved();
  }

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

void appendItem(const TypeCode_base *tc, const tcDescriptor& src, cdrStream& dest);
void fetchItem(const TypeCode_base *tc, cdrStream& src, tcDescriptor& dest);

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
tcParser::copyTcDescriptorToStream(const CORBA::TypeCode_ptr tc,
				   const tcDescriptor &src,
				   cdrStream& dest) {
  appendItem(ToTcBase_Checked(tc), src, dest);
}

void
tcParser::copyStreamToTcDescriptor(const CORBA::TypeCode_ptr tc,
				   cdrStream& src,
				   tcDescriptor& dest) {
  fetchItem(ToTcBase_Checked(tc), src, dest);
}
  
void
tcParser::skip(const CORBA::TypeCode_ptr tc, cdrStream &s) {
  skipUsingTC(ToTcBase_Checked(tc), s);
}

//////////////////////////////////////////////////////////////////////
/////////////////////// Internal Implementation //////////////////////
//////////////////////////////////////////////////////////////////////

static inline CORBA::ULong getEnumData(const tcDescriptor &tcdata) {
  switch (tcdata.p_enum.size) {
  case 1: return *((CORBA::Octet*)tcdata.p_enum.data);
  case 2: return *((CORBA::UShort*)tcdata.p_enum.data);
  default: return *((CORBA::ULong*)tcdata.p_enum.data);
#ifdef HAS_LongLong
  case 8: return *((CORBA::ULongLong*)tcdata.p_enum.data);
#endif
  };
  OMNIORB_ASSERT(0);
  // Fails if compiler has picked sizeof(enum) which is not a power of 2.
}

static inline void setEnumData(tcDescriptor &tcdata,const CORBA::ULong value) {
  switch (tcdata.p_enum.size) {
  case 1: *((CORBA::Octet*)tcdata.p_enum.data) = value; return;
  case 2: *((CORBA::UShort*)tcdata.p_enum.data) = value; return;
  default: *((CORBA::ULong*)tcdata.p_enum.data) = value; return;
#ifdef HAS_LongLong
  case 8: *((CORBA::ULongLong*)tcdata.p_enum.data) = value; return;
#endif
  };
  OMNIORB_ASSERT(0);
  // Fails if compiler has picked sizeof(enum) which is not a power of 2.
}

void appendSimpleItem(CORBA::TCKind tck, const tcDescriptor &tcdata, cdrStream& buf)
{
  switch (tck)
    {
      // SIMPLE TYPES
    case CORBA::tk_short:
      *tcdata.p_short   >>= buf;
      break;
    case CORBA::tk_ushort:
      *tcdata.p_ushort  >>= buf;
      break;
    case CORBA::tk_long:
      *tcdata.p_long    >>= buf;
      break;
    case CORBA::tk_ulong:
      *tcdata.p_ulong   >>= buf;
      break;
    case CORBA::tk_float:
      *tcdata.p_float   >>= buf;
      break;
    case CORBA::tk_double:
      *tcdata.p_double  >>= buf;
      break;
    case CORBA::tk_boolean:
      buf.marshalBoolean(*tcdata.p_boolean);
      break;
    case CORBA::tk_char:
      buf.marshalChar(*tcdata.p_char);
      break;
    case CORBA::tk_wchar:
      buf.marshalWChar(*tcdata.p_wchar);
      break;
    case CORBA::tk_octet:
      buf.marshalOctet(*tcdata.p_octet);
      break;
    case CORBA::tk_enum:
      getEnumData(tcdata) >>= buf;
      break;

#ifdef HAS_LongLong
    case CORBA::tk_longlong:
      *tcdata.p_longlong    >>= buf;
      break;
    case CORBA::tk_ulonglong:
      *tcdata.p_ulonglong   >>= buf;
      break;
#endif
#ifdef HAS_LongDouble
    case CORBA::tk_longdouble:
      *tcdata.p_longdouble  >>= buf;
      break;
#endif

      // OTHER TYPES
    default:
      OMNIORB_ASSERT(0);
    }
}


void appendItem(const TypeCode_base* tc, const tcDescriptor& tcdata, cdrStream& buf)
{
  // Follow any TypeCode_indirect objects created by
  // ORB::create_recursive_tc().
  while (tc->NP_kind() == 0xffffffff) {
    tc = ((TypeCode_indirect*)tc)->NP_resolved();
  }

  // How to marshal the data depends entirely on the TypeCode
  switch (tc->NP_kind()) {

    // TYPES WITH NO DATA TO MARSHAL
  case CORBA::tk_null:
  case CORBA::tk_void:
    break;

    // SIMPLE TYPES
  case CORBA::tk_short:
  case CORBA::tk_ushort:
  case CORBA::tk_long:
  case CORBA::tk_ulong:
  case CORBA::tk_float:
  case CORBA::tk_double:
  case CORBA::tk_boolean:
  case CORBA::tk_char:
  case CORBA::tk_wchar:
  case CORBA::tk_octet:
  case CORBA::tk_enum:
#ifdef HAS_LongLong
  case CORBA::tk_longlong:
  case CORBA::tk_ulonglong:
#endif
#ifdef HAS_LongDouble
  case CORBA::tk_longdouble:
#endif
    appendSimpleItem(tc->NP_kind(), tcdata, buf);
    break;

  case CORBA::tk_any:
    *tcdata.p_any >>= buf;
    break;

    // COMPLEX TYPES
  case CORBA::tk_Principal:
    // For the principal type, the caller passes us a CORBA::PrincipalId,
    // which will directly marshal itself to the wire.
    *tcdata.p_Principal >>= buf;
    break;

  case CORBA::tk_objref:
    // Call the user-defined callback to get the ptr in a suitable form
    CORBA::Object_Helper::
      marshalObjRef(tcdata.p_objref.getObjectPtr(&tcdata.p_objref),
		    buf);
    break;

  case CORBA::tk_TypeCode:
    CORBA::TypeCode::marshalTypeCode(tcdata.p_TypeCode->_ptr, buf);
    break;

  case CORBA::tk_string:
    {
      buf.marshalString(*tcdata.p_string.ptr);
      break;
    }

  case CORBA::tk_wstring:
    {
      buf.marshalWString(*tcdata.p_wstring.ptr);
      break;
    }

  case CORBA::tk_fixed:
    {
      CORBA::Fixed f(*tcdata.p_fixed);
      f.PR_setLimits(tc->fixed_digits(), tc->fixed_scale());
      f >>= buf;
      break;
    }

    // CONSTRUCTED TYPES
  case CORBA::tk_union:
    {
      tcUnionDiscriminatorType     disc_val;
      tcDescriptor                 disc_desc;
      CORBA::PR_unionDiscriminator discrim;

      disc_desc.p_char = (CORBA::Char*) &disc_val;

      // Get a descriptor for the discriminator, and marshal it
      // into the buffer.
      tcdata.p_union.getDiscriminator(&tcdata.p_union, disc_desc, discrim);
      appendSimpleItem(tc->NP_discriminator_type()->NP_kind(), disc_desc, buf);

      // Determine the index of the selected member.
      CORBA::Long index =
	((TypeCode_union*)tc)->NP_index_from_discriminator(discrim);
      if( index < 0 )
	// Implicit default, so no member.
	break;

      // Get the data.
      tcDescriptor data_desc;

      if( !tcdata.p_union.getValueDesc(&tcdata.p_union, data_desc) )
	OMNIORB_ASSERT(0);

      appendItem(tc->NP_member_type(index), data_desc, buf);
      break;
    }

  case CORBA::tk_struct:
    {
      CORBA::ULong nmembers = tc->NP_member_count();

      // Save the individual elements.
      for (CORBA::ULong i=0; i < nmembers; i++)
	{
	  tcDescriptor desc;
	    
	  // Get a descriptor for the member.
	  if( !tcdata.p_struct.getMemberDesc(&tcdata.p_struct, i, desc) )
	    OMNIORB_ASSERT(0);

	  // Append the element to the mbuf.
	  TypeCode_base* tctmp = tc->NP_member_type(i);
	  appendItem(tctmp, desc, buf);
	}
      break;
    }

  case CORBA::tk_except:
    {
      // Exceptions are passed on the wire as repo id followed
      // by members - but for contents of Any we are only interested
      // in members. Therefore we copy the members only here, and
      // the stubs/GIOP_S code deals with the repo id.

      CORBA::ULong nmembers = tc->member_count();

      // Save the individual elements.
      for (CORBA::ULong i=0; i < nmembers; i++)
	{
	  tcDescriptor desc;

	  // Get a descriptor for the member.
	  if( !tcdata.p_except.getMemberDesc(&tcdata.p_except, i, desc) )
	    OMNIORB_ASSERT(0);

	  // Append the element to the mbuf.
	  TypeCode_base* tctmp = tc->NP_member_type(i);
	  appendItem(tctmp, desc, buf);
	}
      break;
    }

  case CORBA::tk_sequence:
    {
      CORBA::ULong max = tcdata.p_sequence.getElementCount(&tcdata.p_sequence);
      TypeCode_base* tctmp = tc->NP_content_type();

      // Save the length of the sequence.
      max >>= buf;

      // Save the sequence data.
      for (CORBA::ULong i=0; i < max; i++)
	{
	  tcDescriptor desc;
	  CORBA::ULong contiguous = 0;

	  // Get a descriptor for the sequence element.
	  if( !tcdata.p_sequence.getElementDesc(&tcdata.p_sequence, i,
						desc, contiguous) )
	    OMNIORB_ASSERT(0);

	  if (contiguous <= 1) {
	    appendItem(tctmp, desc, buf);
	  } else {
	    const TypeCode_alignTable& alignTable = tctmp->alignmentTable();
	    
	    // Elements are in a contiguous block - fast copy them
	    // This assumes:
	    // - Element type is simple & has simple alignment
	    // - Element type preserves its own alignment
	    // - Byteorder matches between RAM and MemBuff
	    // - Pointer to initial element is stored in same
	    //   place as p_streamdata by the compiler (see tcDescriptor)
	    // IF ANY OF THESE AREN'T TRUE, THIS FAILS!!!
	    buf.put_octet_array((CORBA::Char*)desc.p_streamdata,
				    contiguous * (alignTable[0].simple.size),
				    alignTable[0].simple.alignment);
	    i += contiguous;
	  }
	}
      break;
    }

  case CORBA::tk_array:
    {
      CORBA::ULong max = tc->NP_length();
      TypeCode_base* tctmp = tc->NP_content_type();

      for (CORBA::ULong i=0; i < max; i++)
	{
	  tcDescriptor desc;
	  CORBA::ULong contiguous = 0;

	  // Get a descriptor for the array element.
	  if( !tcdata.p_array.getElementDesc(&tcdata.p_array, i,
					     desc, contiguous) )
	    OMNIORB_ASSERT(0);

	  // Append the element to the mbuf.
	  if (contiguous <= 1) {
	    appendItem(tctmp, desc, buf);
	  } else {
	    const TypeCode_alignTable& alignTable = tctmp->alignmentTable();
	    
	    // Elements are in a contiguous block - fast copy them
	    // This assumes:
	    // - Element type is simple & has simple alignment
	    // - Element type preserves its own alignment
	    // - Byteorder matches between RAM and MemBuff
	    // - Pointer to initial element is stored in same
	    //   place as p_streamdata by the compiler (see tcDescriptor)
	    // IF ANY OF THESE AREN'T TRUE, THIS FAILS!!!
	    buf.get_octet_array((CORBA::Char*)desc.p_streamdata,
				   contiguous * (alignTable[0].simple.size),
				   alignTable[0].simple.alignment);
	    i += contiguous;
	  }
	}
      break;
    }

  case CORBA::tk_alias:
    appendItem(tc->NP_content_type(), tcdata, buf);
    break;

  default:
    OMNIORB_ASSERT(0);
  }  // switch (tc->NP_kind()) {
}


void fetchSimpleItem(CORBA::TCKind tck, cdrStream &src, tcDescriptor &tcdata)
{
  switch (tck)
    {

      // SIMPLE TYPES
    case CORBA::tk_short:
      *tcdata.p_short   <<= src;
      break;
    case CORBA::tk_ushort:
      *tcdata.p_ushort  <<= src;
      break;
    case CORBA::tk_long:
      *tcdata.p_long    <<= src;
      break;
    case CORBA::tk_ulong:
      *tcdata.p_ulong   <<= src;
      break;
    case CORBA::tk_float:
      *tcdata.p_float   <<= src;
      break;
    case CORBA::tk_double:
      *tcdata.p_double  <<= src;
      break;
    case CORBA::tk_boolean:
      *tcdata.p_boolean = src.unmarshalBoolean(); 
      break;
    case CORBA::tk_char:
      *tcdata.p_char = src.unmarshalChar();
      break;
    case CORBA::tk_wchar:
      *tcdata.p_wchar = src.unmarshalWChar();
      break;
    case CORBA::tk_octet:
      *tcdata.p_octet = src.unmarshalOctet();
      break;
    case CORBA::tk_enum:
      {
	CORBA::ULong tmp;
	tmp <<= src ;
	setEnumData(tcdata, tmp);
      };
      break;

#ifdef HAS_LongLong
    case CORBA::tk_longlong:
      *tcdata.p_longlong    <<= src;
      break;
    case CORBA::tk_ulonglong:
      *tcdata.p_ulonglong   <<= src;
      break;
#endif
#ifdef HAS_LongDouble
    case CORBA::tk_longdouble:
      *tcdata.p_longdouble  <<= src;
      break;
#endif

      // OTHER TYPES
    default:
      OMNIORB_ASSERT(0);
    }
}


void fetchItem(const TypeCode_base* tc, cdrStream& src, tcDescriptor& tcdata)
{
  // Follow any TypeCode_indirect objects created by
  // ORB::create_recursive_tc().
  while (tc->NP_kind() == 0xffffffff) {
    tc = ((TypeCode_indirect*)tc)->NP_resolved();
  }

  // How to unmarshal the data depends entirely on the TypeCode
  switch( tc->NP_kind() ) {

    // TYPES WITH NO DATA TO MARSHAL
  case CORBA::tk_null:
  case CORBA::tk_void:
    break;

    // SIMPLE TYPES
  case CORBA::tk_short:
  case CORBA::tk_ushort:
  case CORBA::tk_long:
  case CORBA::tk_ulong:
  case CORBA::tk_float:
  case CORBA::tk_double:
  case CORBA::tk_boolean:
  case CORBA::tk_char:
  case CORBA::tk_wchar:
  case CORBA::tk_octet:
  case CORBA::tk_enum:
#ifdef HAS_LongLong
  case CORBA::tk_longlong:
  case CORBA::tk_ulonglong:
#endif
#ifdef HAS_LongDouble
  case CORBA::tk_longdouble:
#endif
    fetchSimpleItem(tc->NP_kind(), src, tcdata);
    break;

  case CORBA::tk_any:
    // Fetch the any.  Note that the caller must have allocated
    // the Any for, already.
    *tcdata.p_any <<= src;
    break;

    // COMPLEX TYPES
  case CORBA::tk_Principal:
    // For the Principal type, we just fill in the supplied
    // CORBA::PrincipalId.
    tcdata.p_Principal = new CORBA::PrincipalID;
    *tcdata.p_Principal <<= src;
    break;

  case CORBA::tk_objref:
    // Call the user-defined callback to set the object reference with
    // appropriate narrowing.
    tcdata.p_objref.setObjectPtr(&tcdata.p_objref,
		    CORBA::Object_Helper::unmarshalObjRef(src));
    break;

  case CORBA::tk_TypeCode:
    // Overwrite the TypeCode_ptr to point to the new typecode object
    *tcdata.p_TypeCode = CORBA::TypeCode::unmarshalTypeCode(src);
    break;

  case CORBA::tk_string:
    {
      if(tcdata.p_string.release )
	_CORBA_String_helper::free(*tcdata.p_string.ptr);
      *tcdata.p_string.ptr = src.unmarshalString();
      break;
    }

  case CORBA::tk_wstring:
    {
      if(tcdata.p_wstring.release )
	_CORBA_WString_helper::free(*tcdata.p_wstring.ptr);
      *tcdata.p_wstring.ptr = src.unmarshalWString();
      break;
    }

  case CORBA::tk_fixed:
    {
      CORBA::Fixed f;
      f.PR_setLimits(tc->fixed_digits(), tc->fixed_scale());
      f <<= src;
      *tcdata.p_fixed = f;
      break;
    }

    // CONSTRUCTED TYPES
  case CORBA::tk_union:
    {
      // Allocate some space to load the discriminator into.
      tcUnionDiscriminatorType disc_val;
      tcDescriptor             disc_desc;

      disc_desc.p_char = (CORBA::Char*) &disc_val;
      fetchSimpleItem(tc->NP_discriminator_type()->NP_kind(), src, disc_desc);

      // Determine the discriminator value.
      CORBA::PR_unionDiscriminator discrim = 0;
      switch( tc->NP_discriminator_type()->NP_kind() ) {
      case CORBA::tk_char:
	discrim = (CORBA::PR_unionDiscriminator) *disc_desc.p_char;
	break;
      case CORBA::tk_boolean:
	discrim = (CORBA::PR_unionDiscriminator) *disc_desc.p_boolean;
	break;
      case CORBA::tk_short:
	discrim = (CORBA::PR_unionDiscriminator) *disc_desc.p_short;
	break;
      case CORBA::tk_ushort:
	discrim = (CORBA::PR_unionDiscriminator) *disc_desc.p_ushort;
	break;
      case CORBA::tk_long:
	discrim = (CORBA::PR_unionDiscriminator) *disc_desc.p_long;
	break;
      case CORBA::tk_ulong:
	discrim = (CORBA::PR_unionDiscriminator) *disc_desc.p_ulong;
	break;
      case CORBA::tk_enum:
	discrim = (CORBA::PR_unionDiscriminator) getEnumData(disc_desc);
	break;
#ifdef HAS_LongLong
      case CORBA::tk_longlong:
	discrim = (CORBA::PR_unionDiscriminator) *disc_desc.p_longlong;
	break;
      case CORBA::tk_ulonglong:
	discrim = (CORBA::PR_unionDiscriminator) *disc_desc.p_ulonglong;
	break;
#endif
      // case CORBA::tk_wchar:
      default:
	OMNIORB_ASSERT(0);
      }

      // Determine the index of the selected member.
      CORBA::Long index =
	((TypeCode_union*)tc)->NP_index_from_discriminator(discrim);

      // Tell the union the new discriminator value.
      tcdata.p_union.setDiscriminator(&tcdata.p_union, discrim,
				      index == tc->NP_default_index());

      if( index < 0 )
	// Implicit default - so no member to fetch
	break;

      // Unmarshal the union data.
      tcDescriptor data_desc;

      if( !tcdata.p_union.getValueDesc(&tcdata.p_union, data_desc) )
	OMNIORB_ASSERT(0);

      fetchItem(tc->NP_member_type(index), src, data_desc);
      break;
    }

  case CORBA::tk_struct:
    {
      CORBA::ULong nmembers = tc->NP_member_count();

      // Load the individual elements.
      for (CORBA::ULong i=0; i < nmembers; i++)
	{
	  tcDescriptor desc;
	    
	  // Get a descriptor for the struct element.
	  if( !tcdata.p_struct.getMemberDesc(&tcdata.p_struct, i, desc) )
	    OMNIORB_ASSERT(0);

	  fetchItem(tc->NP_member_type(i), src, desc);
	}
      break;
    }

  case CORBA::tk_except:
    {
      // Exceptions are passed on the wire as repo id followed
      // by members - but for contents of Any we are only interested
      // in members. Therefore we copy the members only here, and
      // the stubs/GIOP_S code deals with the repo id.

      CORBA::ULong nmembers = tc->NP_member_count();

      // Load the individual elements.
      for (CORBA::ULong i=0; i < nmembers; i++)
	{
	  tcDescriptor desc;

	  if( !tcdata.p_except.getMemberDesc(&tcdata.p_except, i, desc) )
	    OMNIORB_ASSERT(0);

	  fetchItem(tc->NP_member_type(i), src, desc);
	}
      break;
    }

  case CORBA::tk_sequence:
    {
      TypeCode_base* tctmp = tc->NP_content_type();

      // Get the sequence length
      CORBA::ULong nelem;
      nelem <<= src;

      // Allocate space for it
      tcdata.p_sequence.setElementCount(&tcdata.p_sequence, nelem);

      // Load the sequence data
      for (CORBA::ULong i=0; i < nelem; i++)
	{
	  tcDescriptor desc;
	  CORBA::ULong contiguous = 0;

	  if(!tcdata.p_sequence.getElementDesc(&tcdata.p_sequence, i,
						desc, contiguous) )
	    OMNIORB_ASSERT(0);

	  if (contiguous <= 1) {
	    fetchItem(tctmp, src, desc);
	  } else {
	    const TypeCode_alignTable& alignTable = tctmp->alignmentTable();
	   
	    // Elements are in a contiguous block - fast copy them
	    // This assumes:
	    // - Element type is simple & has simple alignment
	    // - Element type preserves its own alignment
	    // - Byteorder matches between RAM and MemBuff
	    // - Pointer to initial element is stored in same
	    //   place as p_streamdata by the compiler (see tcDescriptor)
	    // - Contiguous buffer is large enough & suitably aligned
	    // IF ANY OF THESE AREN'T TRUE, THIS FAILS!!!
	    src.get_octet_array((CORBA::Char*)desc.p_streamdata,
				    contiguous * (alignTable[0].simple.size),
				    alignTable[0].simple.alignment);
	    i += contiguous;
	  }
	}
      break;
    }

  case CORBA::tk_array:
    {
      CORBA::ULong length = tc->NP_length();
      TypeCode_base* tctmp = tc->NP_content_type();

      for (CORBA::ULong i=0; i < length; i++)
	{
	  tcDescriptor desc;
	  CORBA::ULong contiguous = 0;
	    
	  if( !tcdata.p_array.getElementDesc(&tcdata.p_array, i,
					     desc, contiguous) )
	    OMNIORB_ASSERT(0);

	  if (contiguous <= 1) {
	    fetchItem(tctmp, src, desc);
	  } else {
	    const TypeCode_alignTable& alignTable = tctmp->alignmentTable();
	   
	    // Elements are in a contiguous block - fast copy them
	    // This assumes:
	    // - Element type is simple & has simple alignment
	    // - Element type preserves its own alignment
	    // - Byteorder matches between RAM and MemBuff
	    // - Pointer to initial element is stored in same
	    //   place as p_streamdata by the compiler (see tcDescriptor)
	    // - Contiguous buffer is large enough & suitably aligned
	    // IF ANY OF THESE AREN'T TRUE, THIS FAILS!!!
	    src.get_octet_array((CORBA::Char*)desc.p_streamdata,
				    contiguous * (alignTable[0].simple.size),
				    alignTable[0].simple.alignment);
	    i += contiguous;
	  }
	}
      break;
    }

  case CORBA::tk_alias:
    fetchItem(tc->NP_content_type(), src, tcdata);
    break;

  default:
    // Unexpected kind
    OMNIORB_ASSERT(0);
  }
}


//////////////////////////////////////////////////////////////////////
////////////////////// Data Descriptor Functions /////////////////////
//////////////////////////////////////////////////////////////////////

void
_0RL_tcParser_objref_setObjectPtr(const tcObjrefDesc* desc,
				  CORBA::Object_ptr ptr)
{
  CORBA::Object_ptr* pp = (CORBA::Object_ptr*)desc->opq_objref;
  if (desc->opq_release && !CORBA::is_nil(*pp)) {
    CORBA::release(*pp);
  }
  *pp = ptr;
}


CORBA::Object_ptr
_0RL_tcParser_objref_getObjectPtr(const tcObjrefDesc* desc)
{
  return *((CORBA::Object_ptr*)desc->opq_objref);
}

OMNI_NAMESPACE_END(omni)

