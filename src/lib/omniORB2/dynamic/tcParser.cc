// -*- Mode: C++; -*-
//                            Package   : omniORB2
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

/*
  $Log$
  Revision 1.9.4.1  1999/09/15 20:18:22  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

*/

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

inline void fastCopyUsingTC(TypeCode_base* tc, cdrStream& ibuf, cdrStream& obuf)
{
  // This can only be used if both streams have the same
  // byte order.

  const TypeCode_alignTable& alignTbl = tc->alignmentTable();
  unsigned i = 0;  // don't even ask ... just accept it.

  if( alignTbl.entries() == 0 )
    throw omniORB::fatalException(__FILE__,__LINE__,
	"fastCopyUsingTC() - alignment table has zero entries");

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
	  { CORBA::Object_member d; d <<= ibuf; d >>= obuf; break; }

	case CORBA::tk_TypeCode:
	  { CORBA::TypeCode_member d; d <<= ibuf; d >>= obuf; break; }

	case CORBA::tk_string:
	  {
	    CORBA::ULong len;
	    len <<= ibuf;
	    len >>= obuf;
	    ibuf.copy_to(obuf,len);
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
	    // then it will have been deal with above. We only
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
	  throw omniORB::fatalException(__FILE__,__LINE__,
			"fastCopyUsingTC() - unexpected typecode kind");
	}
	break;
      }

    default:
      throw omniORB::fatalException(__FILE__,__LINE__,
		"fastCopyUsingTC() - unknown alignment info type");
    }
  }
}


void copyUsingTC(TypeCode_base* tc, cdrStream& ibuf, cdrStream& obuf)
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
	ibuf.copy_to(obuf,len);
	return;
      }

    case CORBA::tk_objref:
      { CORBA::Object_member d; d <<= ibuf; d >>= obuf; return; }

    case CORBA::tk_TypeCode:
      { CORBA::TypeCode_member d; d <<= ibuf; d >>= obuf; return; }

    case CORBA::tk_string:
      {
	CORBA::ULong len;
	len <<= ibuf;
	len >>= obuf;
	ibuf.copy_to(obuf,len);
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
      throw omniORB::fatalException(__FILE__,__LINE__,
          "tcParser::copyUsingTC() - unknown typecode kind");
    }
}


void skipUsingTC(TypeCode_base* tc, cdrStream& buf)
{
  CORBA::Char dummy;
  const TypeCode_alignTable& alignTbl = tc->alignmentTable();
  unsigned i = 0;  // don't even ask ... just accept it.

  if( alignTbl.entries() == 0 )
    throw omniORB::fatalException(__FILE__,__LINE__,
	"skipUsingTC() - alignment table has zero entries");

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

	case CORBA::tk_objref:
	  { CORBA::Object_member d; d <<= buf; break; }

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

	default:
	  throw omniORB::fatalException(__FILE__,__LINE__,
			"skipUsingTC() - unexpected typecode kind");
	}
	break;
      }

    default:
      throw omniORB::fatalException(__FILE__,__LINE__,
		"skipUsingTC() - unknown alignment info type");
    }
  }
}

//////////////////////////////////////////////////////////////////////
////////////////////////////// tcParser //////////////////////////////
//////////////////////////////////////////////////////////////////////

void
tcParser::copyTo(cdrStream& mbuf, int rewind)
{
  if( rewind )  pd_mbuf.rewindInputPtr();
  if( !pd_mbuf.unmarshal_byte_swap() )
    fastCopyUsingTC(ToTcBase_Checked(pd_tc), pd_mbuf, mbuf);
  else
    copyUsingTC(ToTcBase_Checked(pd_tc), pd_mbuf, mbuf);
}


void
tcParser::copyTo(tcDescriptor& desc, int rewind)
{
  if( rewind )  pd_mbuf.rewindInputPtr();
  fetchItem(ToTcBase_Checked(pd_tc), desc);
}


void
tcParser::copyFrom(cdrStream& mbuf, int flush)
{
  if( flush )  pd_mbuf.rewindPtrs();
  if( mbuf.unmarshal_byte_swap() == pd_mbuf.marshal_byte_swap() )
    fastCopyUsingTC(ToTcBase_Checked(pd_tc), mbuf, pd_mbuf);
  else
    copyUsingTC(ToTcBase_Checked(pd_tc), mbuf, pd_mbuf);
}


void
tcParser::copyFrom(tcDescriptor& desc, int flush)
{
  if( flush )  pd_mbuf.rewindPtrs();
  appendItem(ToTcBase_Checked(pd_tc), desc);
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
tcParser::skip(cdrStream& mbs, CORBA::TypeCode_ptr tc)
{
  skipUsingTC(ToTcBase_Checked(tc), mbs);
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
      throw omniORB::fatalException(__FILE__,__LINE__,
	 "tcParser::appendSimpleItem() - bad typecode kind");
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
      if (*tcdata.p_string) {
	CORBA::ULong len = strlen(*tcdata.p_string) + 1;
	len >>= pd_mbuf;
	pd_mbuf.put_char_array((unsigned char*) *tcdata.p_string, len);
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
	throw omniORB::fatalException(__FILE__,__LINE__,
         "tcParser::appendItem() - failed to get descriptor for union value");

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
	    throw omniORB::fatalException(__FILE__,__LINE__,
		   "tcParser::appendItem() - failed to get descriptor for"
					  " struct member");

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
	    throw omniORB::fatalException(__FILE__,__LINE__,
	      "tcParser::appendItem() - failed to get exception descriptor");

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

	  // Get a descriptor for the sequence element.
	  if( !tcdata.p_sequence.getElementDesc(&tcdata.p_sequence, i, desc) )
	    throw omniORB::fatalException(__FILE__,__LINE__,
	      "tcParser::appendItem() - failed to get sequence descriptor");

	  // Append the element to the mbuf.
	  appendItem(tctmp, desc);
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

	  // Get a descriptor for the array element.
	  if( !tcdata.p_array.getElementDesc(&tcdata.p_array, i, desc) )
	    throw omniORB::fatalException(__FILE__,__LINE__,
	      "tcParser::appendItem() - failed to get array descriptor");

	  // Append the element to the mbuf.
	  appendItem(tctmp, desc);
	}
      break;
    }

  case CORBA::tk_alias:
    appendItem(tc->NP_content_type(), tcdata);
    break;

  default:
    throw omniORB::fatalException(__FILE__,__LINE__,
        "tcParser::appendItem() - unknown typecode kind");
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
      throw omniORB::fatalException(__FILE__,__LINE__,
	 "tcParser::fetchSimpleItem() - bad typecode kind");
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
      if (*tcdata.p_string) {
	CORBA::string_free(*tcdata.p_string);
	*tcdata.p_string = 0;
      }
      CORBA::ULong len;
      len <<= pd_mbuf;
      if (len) {
	*tcdata.p_string = CORBA::string_alloc(len-1);
	pd_mbuf.get_char_array((unsigned char*)*tcdata.p_string,len);
      }
      else {
	*tcdata.p_string = CORBA::string_dup((const char*)"");
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
	throw omniORB::fatalException(__FILE__,__LINE__,
          "tcParser::fetchItem() - illegal disciminator type");
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
	throw omniORB::fatalException(__FILE__,__LINE__,
          "tcParser::fetchItem() - failed to get descriptor for union value");

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
	    throw omniORB::fatalException(__FILE__,__LINE__,
		 "tcParser::fetchItem() - struct member descriptor");

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
	    throw omniORB::fatalException(__FILE__,__LINE__,
		 "tcParser::fetchItem() - exception member descriptor");

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

	  if( !tcdata.p_sequence.getElementDesc(&tcdata.p_sequence, i, desc) )
	    throw omniORB::fatalException(__FILE__,__LINE__,
		 "tcParser::fetchItem() - sequence member descriptor");

	  fetchItem(tctmp, desc);
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
	    
	  if( !tcdata.p_array.getElementDesc(&tcdata.p_array, i, desc) )
	    throw omniORB::fatalException(__FILE__,__LINE__,
		 "tcParser::fetchItem() - struct member descriptor");

	  fetchItem(tctmp, desc);
	}
      break;
    }

  case CORBA::tk_alias:
    fetchItem(tc->NP_content_type(), tcdata);
    break;

  default:
    throw omniORB::fatalException(__FILE__,__LINE__,
        "tcParser::fetchItem() - unknown typecode kind");
  }  // switch( tc->NP_kind() ) {
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

