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


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//////////////////////////////////////////////////////////////////////
/////////////////////// Internal Implementation //////////////////////
//////////////////////////////////////////////////////////////////////

//?? Further optimisations possible if source and destination have
// the same alignment at any point - 'cos we can then just byte
// copy EVERYTHING from there onwards.
// ie. if( src_posn % 8 == dst_posn % 8 ) ...
//  The only interesting question is knowing when to stop ...

template <class ibuf_t, class obuf_t>
inline void fastCopyUsingTC(TypeCode_base* tc, ibuf_t& ibuf, obuf_t& obuf)
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
      obuf.copy_from(ibuf, alignTbl[i].simple.size,
		     alignTbl[i].simple.alignment);
      break;

    case TypeCode_alignTable::it_nasty:
      { // Data is variable length or has variable alignment requirements.
	tc = alignTbl[i].nasty.tc;

	switch( tc->NP_kind() ) {

	  //?? Some of these could be faster (Any, TypeCode, objref ...)
	case CORBA::tk_any:
	  { CORBA::Any d; d <<= ibuf; d >>= obuf; break; }

	case CORBA::tk_Principal:
	  {
	    CORBA::ULong len;
	    len <<= ibuf;
	    len >>= obuf;
	    obuf.copy_from(ibuf, len);
	    break;
	  }

	case CORBA::tk_objref:
	  { CORBA::Object_Member d; d <<= ibuf; d >>= obuf; break; }

	case CORBA::tk_TypeCode:
	  { CORBA::TypeCode_member d; d <<= ibuf; d >>= obuf; break; }

	case CORBA::tk_string:
	  {
	    CORBA::ULong len;
	    len <<= ibuf;
	    len >>= obuf;
	    obuf.copy_from(ibuf, len);
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
	      obuf.copy_from(ibuf, eat[0].simple.size +
			     (length - 1) * size_aligned,
			     eat[0].simple.alignment);
	    }
	    else if( eat.has_only_simple() ) {
	      // Copy the first element ...
	      CORBA::ULong start = 0;
	      for( unsigned j = 0; j < eat.entries(); j++ ) {
		start = omni::align_to(start, eat[j].simple.alignment);
		start += eat[j].simple.size;
		obuf.copy_from(ibuf, eat[j].simple.size,
			       eat[j].simple.alignment);
	      }
	      // Calculate the size of subsequent elements ...
	      CORBA::ULong end = start;
	      for( unsigned k = 0; k < eat.entries(); k++ ) {
		end = omni::align_to(end, eat[k].simple.alignment);
		end += eat[k].simple.size;
	      }
	      // ... then copy the rest as a block.
	      obuf.copy_from(ibuf, (length - 1) * (end - start));
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


template <class ibuf_t, class obuf_t>
inline void copyUsingTC(TypeCode_base* tc, ibuf_t& ibuf, obuf_t& obuf)
{
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
      { CORBA::Boolean d; d <<= ibuf; d >>= obuf; return; }
    case CORBA::tk_char:
      { CORBA::Char d;    d <<= ibuf; d >>= obuf; return; }
    case CORBA::tk_octet:
      { CORBA::Octet d;   d <<= ibuf; d >>= obuf; return; }
    case CORBA::tk_enum:
      { CORBA::ULong d;   d <<= ibuf; d >>= obuf; return; }

    case CORBA::tk_any:
      { CORBA::Any d;     d <<= ibuf; d >>= obuf; return; }

    // COMPLEX TYPES
    case CORBA::tk_Principal:
      {
	CORBA::ULong len;
	len <<= ibuf;
	len >>= obuf;
	obuf.copy_from(ibuf, len);
	return;
      }

    case CORBA::tk_objref:
      { CORBA::Object_Member d; d <<= ibuf; d >>= obuf; return; }

    case CORBA::tk_TypeCode:
      { CORBA::TypeCode_member d; d <<= ibuf; d >>= obuf; return; }

    case CORBA::tk_string:
      {
	CORBA::ULong len;
	len <<= ibuf;
	len >>= obuf;
	obuf.copy_from(ibuf, len);
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

    default:
      OMNIORB_ASSERT(0);
    }
}


template <class buf_t>
inline void skipUsingTC(TypeCode_base* tc, buf_t& buf)
{
  CORBA::Char dummy;
  const TypeCode_alignTable& alignTbl = tc->alignmentTable();
  unsigned i = 0;  // don't even ask ... just accept it.

  OMNIORB_ASSERT(alignTbl.entries() > 0);

  for( i = 0; i < alignTbl.entries(); i++ ) {

    switch( alignTbl[i].type ) {
    case TypeCode_alignTable::it_simple:
      buf.get_char_array(&dummy, 0, alignTbl[i].simple.alignment);
      buf.skip(alignTbl[i].simple.size);
      break;

    case TypeCode_alignTable::it_nasty:
      {
	tc = alignTbl[i].nasty.tc;

	switch( tc->NP_kind() ) {

	  //?? Some of these could be faster (Any, TypeCode, objref ...)
	case CORBA::tk_any:
	  { CORBA::Any d; d <<= buf; break; }

	case CORBA::tk_Principal:
	case CORBA::tk_string:
	  {
	    CORBA::ULong len;
	    len <<= buf;
	    buf.skip(len);
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
	      buf.get_char_array(&dummy, 0, eat[0].simple.alignment);
	      buf.skip(eat[0].simple.size + (length - 1) * size_aligned);
	    }
	    else if( eat.has_only_simple() ) {
	      // Skip the first element ...
	      CORBA::ULong start = 0;
	      for( unsigned j = 0; j < eat.entries(); j++ ) {
		start = omni::align_to(start, eat[j].simple.alignment);
		start += eat[j].simple.size;
		buf.get_char_array(&dummy, 0, eat[j].simple.alignment);
		buf.skip(eat[j].simple.size);
	      }
	      // Calculate the size of subsequent elements ...
	      CORBA::ULong end = start;
	      for( unsigned k = 0; k < eat.entries(); k++ ) {
		end = omni::align_to(end, eat[k].simple.alignment);
		end += eat[k].simple.size;
	      }
	      // ... then skip the rest as a block.
	      buf.skip((length - 1) * (end - start));
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
tcParser::copyTo(MemBufferedStream& mbuf, int rewind)
{
  if( rewind )  pd_mbuf.rewind_in_mkr();
  if( omni::myByteOrder == pd_mbuf.RdMessageByteOrder() )
    fastCopyUsingTC(ToTcBase_Checked(pd_tc), pd_mbuf, mbuf);
  else
    copyUsingTC(ToTcBase_Checked(pd_tc), pd_mbuf, mbuf);
}


void
tcParser::copyTo(NetBufferedStream& nbuf, int rewind)
{
  if( rewind )  pd_mbuf.rewind_in_mkr();
  if( omni::myByteOrder == pd_mbuf.RdMessageByteOrder() )
    fastCopyUsingTC(ToTcBase_Checked(pd_tc), pd_mbuf, nbuf);
  else
    copyUsingTC(ToTcBase_Checked(pd_tc), pd_mbuf, nbuf);
}


void
tcParser::copyTo(tcDescriptor& desc, int rewind)
{
  if( rewind )  pd_mbuf.rewind_in_mkr();
  fetchItem(ToTcBase_Checked(pd_tc), desc);
}


void
tcParser::copyFrom(MemBufferedStream& mbuf, int flush)
{
  if( flush )  pd_mbuf.rewind_inout_mkr();
  if( mbuf.RdMessageByteOrder() == pd_mbuf.RdMessageByteOrder() )
    fastCopyUsingTC(ToTcBase_Checked(pd_tc), mbuf, pd_mbuf);
  else
    copyUsingTC(ToTcBase_Checked(pd_tc), mbuf, pd_mbuf);
}


void
tcParser::copyFrom(NetBufferedStream& nbuf, int flush)
{
  if( flush )  pd_mbuf.rewind_inout_mkr();
  if( nbuf.RdMessageByteOrder() == pd_mbuf.RdMessageByteOrder() )
    fastCopyUsingTC(ToTcBase_Checked(pd_tc), nbuf, pd_mbuf);
  else
    copyUsingTC(ToTcBase_Checked(pd_tc), nbuf, pd_mbuf);
}


void
tcParser::copyFrom(tcDescriptor& desc, int flush)
{
  if( flush )  pd_mbuf.rewind_inout_mkr();
  appendItem(ToTcBase_Checked(pd_tc), desc);
}


size_t
tcParser::alignedSize(size_t initialoffset)
{
  pd_mbuf.rewind_in_mkr();

  // MemBufferedStreams are 8 byte aligned, so if <initialoffset> is
  // also 8 byte aligned, we can just use the size of the buffer ...
  if( (initialoffset & 7) == 0 )
    return initialoffset + pd_mbuf.RdMessageUnRead();
  else
    return calculateItemSize(ToTcBase_Checked(pd_tc), initialoffset);
}


CORBA::TypeCode_ptr
tcParser::getTC() const
{
  return pd_tc;
}

void
tcParser::replaceTC(CORBA::TypeCode_ptr tc)
{
  if (pd_tc->equivalent(tc)) {
    pd_tc = CORBA::TypeCode::_duplicate(tc);
  }
  else {
    throw CORBA::TypeCode::BadKind();
  }
}

void
tcParser::setTC_and_reset(CORBA::TypeCode_ptr tc)
{
  // Clear the buffer & change the typecode
  pd_mbuf.rewind_inout_mkr();
  pd_tc = CORBA::TypeCode::_duplicate(tc);
}

void
tcParser::skip(MemBufferedStream& mbs, CORBA::TypeCode_ptr tc)
{
  skipUsingTC(ToTcBase_Checked(tc), mbs);
}


void
tcParser::skip(NetBufferedStream& nbs, CORBA::TypeCode_ptr tc)
{
  skipUsingTC(ToTcBase_Checked(tc), nbs);
}

//////////////////////////////////////////////////////////////////////
/////////////////////// Internal Implementation //////////////////////
//////////////////////////////////////////////////////////////////////

void
tcParser::appendSimpleItem(CORBA::TCKind tck, tcDescriptor &tcdata)
{
  switch (tck)
    {
      // SIMPLE TYPES
    case CORBA::tk_short:
      *tcdata.p_short   >>= pd_mbuf;
      break;
    case CORBA::tk_ushort:
      *tcdata.p_ushort  >>= pd_mbuf;
      break;
    case CORBA::tk_long:
      *tcdata.p_long    >>= pd_mbuf;
      break;
    case CORBA::tk_ulong:
      *tcdata.p_ulong   >>= pd_mbuf;
      break;
    case CORBA::tk_float:
      *tcdata.p_float   >>= pd_mbuf;
      break;
    case CORBA::tk_double:
      *tcdata.p_double  >>= pd_mbuf;
      break;
    case CORBA::tk_boolean:
      *tcdata.p_boolean >>= pd_mbuf;
      break;
    case CORBA::tk_char:
      *tcdata.p_char    >>= pd_mbuf;
      break;
    case CORBA::tk_octet:
      *tcdata.p_octet   >>= pd_mbuf;
      break;
    case CORBA::tk_enum:
      *tcdata.p_enum    >>= pd_mbuf;
      break;

      // OTHER TYPES
    default:
      OMNIORB_ASSERT(0);
    }
}


void
tcParser::appendItem(TypeCode_base* tc, tcDescriptor& tcdata)
{
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
  case CORBA::tk_octet:
  case CORBA::tk_enum:
    appendSimpleItem(tc->NP_kind(), tcdata);
    break;

  case CORBA::tk_any:
    *tcdata.p_any >>= pd_mbuf;
    break;

    // COMPLEX TYPES
  case CORBA::tk_Principal:
    // For the principal type, the caller passes us a CORBA::PrincipalId,
    // which will directly marshal itself to the wire.
    *tcdata.p_Principal >>= pd_mbuf;
    break;

  case CORBA::tk_objref:
    // Call the user-defined callback to get the ptr in a suitable form
    CORBA::Object_Helper::
      marshalObjRef(tcdata.p_objref.getObjectPtr(&tcdata.p_objref),
		    pd_mbuf);
    break;

  case CORBA::tk_TypeCode:
    CORBA::TypeCode::marshalTypeCode(tcdata.p_TypeCode->_ptr, pd_mbuf);
    break;

  case CORBA::tk_string:
    {
      if (*tcdata.p_string.ptr) {
	CORBA::ULong len = strlen(*tcdata.p_string.ptr) + 1;
	len >>= pd_mbuf;
	pd_mbuf.put_char_array((unsigned char*) *tcdata.p_string.ptr, len);
      }
      else {
	CORBA::ULong len = 1;
	len >>= pd_mbuf;
	CORBA::Char(0) >>= pd_mbuf;
	if( omniORB::traceLevel > 1 )  _CORBA_null_string_ptr(0);	
      }
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
      appendSimpleItem(tc->NP_discriminator_type()->NP_kind(), disc_desc);

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

      appendItem(tc->NP_member_type(index), data_desc);
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
	  appendItem(tctmp, desc);
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
	  appendItem(tctmp, desc);
	}
      break;
    }

  case CORBA::tk_sequence:
    {
      CORBA::ULong max = tcdata.p_sequence.getElementCount(&tcdata.p_sequence);
      TypeCode_base* tctmp = tc->NP_content_type();

      // Save the length of the sequence.
      max >>= pd_mbuf;

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
	    appendItem(tctmp, desc);
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
	    pd_mbuf.put_char_array((CORBA::Char*)desc.p_streamdata,
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
	    appendItem(tctmp, desc);
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
	    pd_mbuf.get_char_array((CORBA::Char*)desc.p_streamdata,
				   contiguous * (alignTable[0].simple.size),
				   alignTable[0].simple.alignment);
	    i += contiguous;
	  }
	}
      break;
    }

  case CORBA::tk_alias:
    appendItem(tc->NP_content_type(), tcdata);
    break;

  default:
    OMNIORB_ASSERT(0);
  }  // switch (tc->NP_kind()) {
}


void
tcParser::fetchSimpleItem(CORBA::TCKind tck, tcDescriptor &tcdata)
{
  switch (tck)
    {

      // SIMPLE TYPES
    case CORBA::tk_short:
      *tcdata.p_short   <<= pd_mbuf;
      break;
    case CORBA::tk_ushort:
      *tcdata.p_ushort  <<= pd_mbuf;
      break;
    case CORBA::tk_long:
      *tcdata.p_long    <<= pd_mbuf;
      break;
    case CORBA::tk_ulong:
      *tcdata.p_ulong   <<= pd_mbuf;
      break;
    case CORBA::tk_float:
      *tcdata.p_float   <<= pd_mbuf;
      break;
    case CORBA::tk_double:
      *tcdata.p_double  <<= pd_mbuf;
      break;
    case CORBA::tk_boolean:
      *tcdata.p_boolean <<= pd_mbuf;
      break;
    case CORBA::tk_char:
      *tcdata.p_char    <<= pd_mbuf;
      break;
    case CORBA::tk_octet:
      *tcdata.p_octet   <<= pd_mbuf;
      break;
    case CORBA::tk_enum:
      *tcdata.p_enum    <<= pd_mbuf;
      break;

      // OTHER TYPES
    default:
      OMNIORB_ASSERT(0);
    }
}


void
tcParser::fetchItem(TypeCode_base* tc, tcDescriptor& tcdata)
{
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
  case CORBA::tk_octet:
  case CORBA::tk_enum:
    fetchSimpleItem(tc->NP_kind(), tcdata);
    break;

  case CORBA::tk_any:
    // Fetch the any.  Note that the caller must have allocated
    // the Any for, already.
    *tcdata.p_any <<= pd_mbuf;
    break;

    // COMPLEX TYPES
  case CORBA::tk_Principal:
    // For the Principal type, we just fill in the supplied
    // CORBA::PrincipalId.
    tcdata.p_Principal = new CORBA::PrincipalID;
    *tcdata.p_Principal <<= pd_mbuf;
    break;

  case CORBA::tk_objref:
    // Call the user-defined callback to set the object reference with
    // appropriate narrowing.
    tcdata.p_objref.setObjectPtr(&tcdata.p_objref,
		    CORBA::Object_Helper::unmarshalObjRef(pd_mbuf));
    break;

  case CORBA::tk_TypeCode:
    // Overwrite the TypeCode_ptr to point to the new typecode object
    *tcdata.p_TypeCode = CORBA::TypeCode::unmarshalTypeCode(pd_mbuf);
    break;

  case CORBA::tk_string:
    {
      if(tcdata.p_string.release )
	omni::freeString(*tcdata.p_string.ptr);
      CORBA::ULong len;
      len <<= pd_mbuf;
      if( len ) {
	*tcdata.p_string.ptr = omni::allocString(len - 1);
	pd_mbuf.get_char_array((unsigned char*) *tcdata.p_string.ptr, len);
      }
      else {
	*tcdata.p_string.ptr = (char*) omni::empty_string;
      }
      break;
    }

    // CONSTRUCTED TYPES
  case CORBA::tk_union:
    {
      // Allocate some space to load the discriminator into.
      tcUnionDiscriminatorType disc_val;
      tcDescriptor             disc_desc;

      disc_desc.p_char = (CORBA::Char*) &disc_val;
      fetchSimpleItem(tc->NP_discriminator_type()->NP_kind(), disc_desc);

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
	discrim = (CORBA::PR_unionDiscriminator) *disc_desc.p_enum;
	break;
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

      fetchItem(tc->NP_member_type(index), data_desc);
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

	  fetchItem(tc->NP_member_type(i), desc);
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

	  fetchItem(tc->NP_member_type(i), desc);
	}
      break;
    }

  case CORBA::tk_sequence:
    {
      TypeCode_base* tctmp = tc->NP_content_type();

      // Get the sequence length
      CORBA::ULong nelem;
      nelem <<= pd_mbuf;

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
	    fetchItem(tctmp, desc);
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
	    pd_mbuf.get_char_array((CORBA::Char*)desc.p_streamdata,
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
	    fetchItem(tctmp, desc);
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
	    pd_mbuf.get_char_array((CORBA::Char*)desc.p_streamdata,
				   contiguous * (alignTable[0].simple.size),
				   alignTable[0].simple.alignment);
	    i += contiguous;
	  }
	}
      break;
    }

  case CORBA::tk_alias:
    fetchItem(tc->NP_content_type(), tcdata);
    break;

  default:
    OMNIORB_ASSERT(0);
  }  // switch( tc->NP_kind() ) {
}


size_t
tcParser::calculateItemSize(const TypeCode_base*tc, size_t offset)
{
  CORBA::Char dummy;
  const TypeCode_alignTable& alignTbl = tc->alignmentTable();
  unsigned at_i = 0;  // don't even ask ... just accept it.

  OMNIORB_ASSERT(alignTbl.entries() > 0);

  for( at_i = 0; at_i < alignTbl.entries(); at_i++ ) {

    switch( alignTbl[at_i].type ) {
    case TypeCode_alignTable::it_simple:
      offset = omni::align_to(offset, alignTbl[at_i].simple.alignment) +
	alignTbl[at_i].simple.size;
      pd_mbuf.get_char_array(&dummy, 0, alignTbl[at_i].simple.alignment);
      pd_mbuf.skip(alignTbl[at_i].simple.size);
      break;

    case TypeCode_alignTable::it_nasty:
      {
	tc = alignTbl[at_i].nasty.tc;

	switch( tc->NP_kind() ) {

	  //?? Some of these could be faster (Any, TypeCode, objref ...)
	case CORBA::tk_any:
	  {
	    CORBA::Any tmp;
	    tmp <<= pd_mbuf;
	    offset = tmp._NP_alignedSize(offset);
	    break;
	  }

	case CORBA::tk_Principal:
	case CORBA::tk_string:
	  {
	    CORBA::ULong len;
	    len <<= pd_mbuf;
	    pd_mbuf.skip(len);
	    offset = omni::align_to(offset, omni::ALIGN_4) + 4 + len;
	    break;
	  }

	case CORBA::tk_objref:
	  {
	    CORBA::Object_Member tmp;
	    tmp <<= pd_mbuf;
	    offset = tmp._NP_alignedSize(offset);
	    break;
	  }

	case CORBA::tk_TypeCode:
	  {
	    CORBA::TypeCode_member tmp;
	    tmp <<= pd_mbuf;
	    offset = tmp._NP_alignedSize(offset);
	    break;
	  }

	case CORBA::tk_union:
	  {
	    // Fetch the discriminator value.
	    TypeCode_base* discrimTC = tc->NP_discriminator_type();
	    TypeCode_union::Discriminator discrim =
	      TypeCode_union_helper::unmarshalLabel(discrimTC, pd_mbuf);

	    offset = TypeCode_union_helper::labelAlignedSize(offset,
							     discrimTC);

	    // Skip the value, using the type for the selected member.
	    CORBA::Long index =
	      ((TypeCode_union*)tc)->NP_index_from_discriminator(discrim);
	    if( index >= 0 )
	      offset = calculateItemSize(tc->NP_member_type(index), offset);
	    else
	      ; // Implicit default -- so no member.
	    break;
	  }

	case CORBA::tk_sequence:
	  {
	    CORBA::ULong length; length <<= pd_mbuf;
	    offset = omni::align_to(offset, omni::ALIGN_4) + 4;
	    if( !length )  break;

	    TypeCode_base* elem_tc = tc->NP_content_type();
	    const TypeCode_alignTable& eat = elem_tc->alignmentTable();

	    if( eat.is_simple() ) {
	      // Skip as a single block.
	      CORBA::ULong size_aligned =
		omni::align_to(eat[0].simple.size, eat[0].simple.alignment);
	      pd_mbuf.get_char_array(&dummy, 0, eat[0].simple.alignment);
	      pd_mbuf.skip(eat[0].simple.size + (length - 1) * size_aligned);
	      offset = omni::align_to(offset, eat[0].simple.alignment);
	      offset += eat[0].simple.size + (length - 1) * size_aligned;
	    }
	    else if( eat.has_only_simple() ) {
	      // Skip the first element ...
	      CORBA::ULong start = 0;
	      for( unsigned j = 0; j < eat.entries(); j++ ) {
		start = omni::align_to(start, eat[j].simple.alignment);
		start += eat[j].simple.size;
		pd_mbuf.get_char_array(&dummy, 0, eat[j].simple.alignment);
		pd_mbuf.skip(eat[j].simple.size);
		offset = omni::align_to(offset, eat[j].simple.alignment);
		offset += eat[j].simple.size;
	      }
	      // Calculate the size of subsequent elements ...
	      CORBA::ULong end = start;
	      for( unsigned k = 0; k < eat.entries(); k++ ) {
		end = omni::align_to(end, eat[k].simple.alignment);
		end += eat[k].simple.size;
	      }
	      // ... then skip the rest as a block.
	      pd_mbuf.skip((length - 1) * (end - start));
	      offset += (length - 1) * (end - start);
	    }
	    else {
	      // We can't do better than skipping element by element.
	      for( CORBA::ULong j = 0; j < length; j++ )
		offset = calculateItemSize(elem_tc, offset);
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
	      offset = calculateItemSize(elemTC, offset);

	    break;
	  }

	case CORBA::tk_alias:
	  offset = calculateItemSize(tc->NP_content_type(), offset);
	  break;

	default:
	  OMNIORB_ASSERT(0);
	}
	break;
      }

    default:
      OMNIORB_ASSERT(0);
    }
  }

  return offset;
}

//////////////////////////////////////////////////////////////////////
////////////////////// Data Descriptor Functions /////////////////////
//////////////////////////////////////////////////////////////////////

void
_0RL_tcParser_objref_setObjectPtr(tcObjrefDesc* desc, CORBA::Object_ptr ptr)
{
  CORBA::Object_ptr* pp = (CORBA::Object_ptr*)desc->opq_objref;
  if (desc->opq_release && !CORBA::is_nil(*pp)) {
    CORBA::release(*pp);
  }
  *pp = ptr;
}


CORBA::Object_ptr
_0RL_tcParser_objref_getObjectPtr(tcObjrefDesc* desc)
{
  return *((CORBA::Object_ptr*)desc->opq_objref);
}

