// -*- Mode: C++; -*-
//                            Package   : omniORB2
// tcParseEngine.cc           Created on: 21/10/97
//                            Author    : Eoin Carroll (ewc)
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
//     Implementation of TypeCode parser


/* $Log$
/* Revision 1.3  1998/08/05 17:59:11  sll
/* Added some comment to document the implementation.
/* Fixed bug in parseUnion() which did not handle the case when a union
/* discriminant is set to select the implicit default branch.
/*
 * Revision 1.2  1998/04/07 19:37:54  sll
 * Remove iostream.h
 *
// Revision 1.1  1998/01/27  15:49:07  ewc
// Initial revision
//
 */

#include <string.h>

#include "tcParseEngine.h"



CORBA::ULong 
tcParseEngine::parse(MemBufferedStream& outBuf, CORBA::Boolean reAlign)
{
  pd_reAlign = reAlign;
  CORBA::TCKind tck = pd_tck;
  CORBA::Boolean bOrder = pd_tmpParam.byteOrder();

  try
    {
      parseTC(outBuf);
    }
  catch(...)
    {
      pd_tck = tck;
      pd_tmpParam.rewind_in_mkr();
      unsetByteOrder(bOrder);
      throw;
    }

  // XXX - What about pd_reAlign? Shall we restore it as well?
  //                                                   - SLL
  pd_tck = tck;
  pd_tmpParam.rewind_in_mkr();
  unsetByteOrder(bOrder);

  return outBuf.alreadyWritten();
}


/**************************************************************************/



CORBA::Char* 
tcParseEngine::getUnionLabel(CORBA::TCKind tck, NetBufferedStream& buf, 
			     CORBA::ULong& discrimLen)
{
  CORBA::Char* label;

  switch (tck)
    {
    case CORBA::tk_boolean:
    case CORBA::tk_char:
      {
	if (buf.overrun(1))
	  throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
	label = new CORBA::Char[1];
	try
	  {
	    CORBA::Char contents = 0;
	    contents <<= buf;
	    memcpy((void*)label,(void*)&contents,1);
	  }
	catch(...)
	  {
	    delete[] label;
	    throw;
	  }

	discrimLen = 1;
	break;
      }

    case CORBA::tk_short:
    case CORBA::tk_ushort:
      {
	if (buf.overrun(2))
	  throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
	label = new CORBA::Char[2];
	try
	  {
	    CORBA::UShort contents = 0;
	    contents <<= buf;
	    memcpy((void*)label,(void*)&contents,2);
	  }
	catch(...)
	  {
	    delete[] label;
	    throw;
	  }
	discrimLen = 2;
	break;
      }
			
    case CORBA::tk_long:
    case CORBA::tk_ulong:
    case CORBA::tk_enum:
      {
	if (buf.overrun(4))
	  throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
	label = new CORBA::Char[4];
	try
	  {
	    CORBA::ULong contents = 0;
	    contents <<= buf;
	    memcpy((void*)label,(void*)&contents,4);
	  }

	catch(...)
	  {
	    delete[] label;
	    throw;
	  }
	discrimLen = 4;
	break;
      }
			
    default:
      throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
    };

  return label;
}  


CORBA::Char* 
tcParseEngine::getUnionLabel(CORBA::TCKind tck, MemBufferedStream& buf, 
			     CORBA::ULong& discrimLen)
{
  CORBA::Char* label;

  switch (tck)
    {
    case CORBA::tk_boolean:
    case CORBA::tk_char:
      {
	if (buf.overrun(1))
	  throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
	label = new CORBA::Char[1];
	try
	  {
	    CORBA::Char contents = 0;
	    contents <<= buf;
	    memcpy((void*)label,(void*)&contents,1);
	  }
	catch(...)
	  {
	    delete[] label;
	    throw;
	  }

	discrimLen = 1;
	break;
      }

    case CORBA::tk_short:
    case CORBA::tk_ushort:
      {
	if (buf.overrun(2))
	  throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
	label = new CORBA::Char[2];
	try
	  {
	    CORBA::UShort contents = 0;
	    contents <<= buf;
	    memcpy((void*)label,(void*)&contents,2);
	  }
	catch(...)
	  {
	    delete[] label;
	    throw;
	  }
	discrimLen = 2;
	break;
      }
			
    case CORBA::tk_long:
    case CORBA::tk_ulong:
    case CORBA::tk_enum:
      {
	if (buf.overrun(4))
	  throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
	label = new CORBA::Char[4];
	try
	  {
	    CORBA::ULong contents = 0;
	    contents <<= buf;
	    memcpy((void*)label,(void*)&contents,4);
	  }

	catch(...)
	  {
	    delete[] label;
	    throw;
	  }
	discrimLen = 4;
	break;
      }
			
    default:
      throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
    };

  return label;
}  



/**************************************************************************/



void 
tcParseEngine::parseTCn(MemBufferedStream& outBuf)
{
  switch(pd_tck)
    {
    case CORBA::tk_null:
    case CORBA::tk_void:
      break;
      

    case CORBA::tk_char:
    case CORBA::tk_octet:
    case CORBA::tk_boolean:
      {
	CORBA::Char a;
	a <<= (*pd_nInBuf);
	a >>= outBuf;
	break;
      }
      

    case CORBA::tk_short:
    case CORBA::tk_ushort:
      {
	CORBA::UShort u;
	u <<= (*pd_nInBuf);
	u >>= outBuf;
	break;
      }
      

    case CORBA::tk_long:
    case CORBA::tk_ulong:
      {
	CORBA::ULong l;
	l <<= (*pd_nInBuf);
	l >>= outBuf;
	break;
      }


    case CORBA::tk_float:
      {
#if defined(NO_FLOAT)
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
#else
	CORBA::Float f;
	f <<= (*pd_nInBuf);
	f >>= outBuf;
#endif
	break;
      }


    case CORBA::tk_double:
#if defined(NO_FLOAT)
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
#else
      if (!pd_reAlign)
	{
	  CORBA::Double d;
	  d <<= (*pd_nInBuf);
	  d >>= outBuf;
	}
      else parseDouble(outBuf);
#endif
      break;
      

    case CORBA::tk_any:
      parseAny(outBuf);
      break;
      
      
    case CORBA::tk_Principal:
      parsePrincipal(outBuf);
      break;
      
      
    case CORBA::tk_TypeCode:
      {
	CORBA::TypeCode tmpTC(CORBA::tk_null);
	tmpTC <<= (*pd_nInBuf);
	tmpTC >>= outBuf;
	break;
      }
      
      
    case CORBA::tk_objref:
      parseObjectRef(outBuf);
      break;

      
    case CORBA::tk_struct:
    case CORBA::tk_except:
      parseStructExcept(outBuf);
      break;


    case CORBA::tk_union:
      parseUnion(outBuf);
      break;

      
    case CORBA::tk_enum:
      parseEnum(outBuf);
      break;

    case CORBA::tk_string:
      parseString(outBuf);
      break;

    case CORBA::tk_sequence:
    case CORBA::tk_array:
      parseSeqArray(outBuf);
      break;


    case CORBA::tk_alias:
      parseAlias(outBuf);
      break;


    default:
      throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      break;
    }
}


void 
tcParseEngine::parseTCm(MemBufferedStream& outBuf)
{
  switch(pd_tck)
    {
    case CORBA::tk_null:
    case CORBA::tk_void:
      break;
      

    case CORBA::tk_char:
    case CORBA::tk_octet:
    case CORBA::tk_boolean:
      {
	CORBA::Char a;
	a <<= (*pd_mInBuf);
	a >>= outBuf;
	break;
      }
      

    case CORBA::tk_short:
    case CORBA::tk_ushort:
      {
	CORBA::UShort u;
	u <<= (*pd_mInBuf);
	u >>= outBuf;
	break;
      }
      

    case CORBA::tk_long:
    case CORBA::tk_ulong:
      {
	CORBA::ULong l;
	l <<= (*pd_mInBuf);
	l >>= outBuf;
	break;
      }


    case CORBA::tk_float:
      {
#if defined(NO_FLOAT)
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
#else
	CORBA::Float f;
	f <<= (*pd_mInBuf);
	f >>= outBuf;
#endif
	break;
      }


    case CORBA::tk_double:
#if defined(NO_FLOAT)
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
#else
      if (!pd_reAlign)
	{
	  CORBA::Double d;
	  d <<= (*pd_mInBuf);
	  d >>= outBuf;
	}
      else parseDouble(outBuf);
#endif
      break;
      

    case CORBA::tk_any:
      parseAny(outBuf);
      break;
      
      
    case CORBA::tk_Principal:
      parsePrincipal(outBuf);
      break;
      
      
    case CORBA::tk_TypeCode:
      {
	CORBA::TypeCode tmpTC(CORBA::tk_null);
	tmpTC <<= (*pd_mInBuf);
	tmpTC >>= outBuf;
	break;
      }
      
      
    case CORBA::tk_objref:
      parseObjectRef(outBuf);
      break;

      
    case CORBA::tk_struct:
    case CORBA::tk_except:
      parseStructExcept(outBuf);
      break;


    case CORBA::tk_union:
      parseUnion(outBuf);
      break;

      
    case CORBA::tk_enum:
      parseEnum(outBuf);
      break;

    case CORBA::tk_string:
      parseString(outBuf);
      break;
      
    case CORBA::tk_sequence:
    case CORBA::tk_array:
      parseSeqArray(outBuf);
      break;


    case CORBA::tk_alias:
      parseAlias(outBuf);
      break;


    default:
      throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      break;
    }
}



#if !defined(NO_FLOAT)
void tcParseEngine::parseDouble(MemBufferedStream& outBuf)
{
  if (outBuf.byteOrder() != omni::myByteOrder)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

  CORBA::ULong currAlign = (outBuf.alreadyWritten() + 4) %8;
	  
  if (currAlign > 0)
    {
      currAlign = 8 - currAlign;
      CORBA::Char* filler = 0;
      filler = new CORBA::Char[currAlign];
      if(!filler) throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
      try
	{
	  outBuf.put_char_array(filler,currAlign);
	}
      catch(...)
	{
	  delete[] filler;
	  throw;
	}
      delete[] filler;
    }

  CORBA::Double d;
  
  if (pd_bufType) d <<= (*pd_nInBuf);
  else d <<= (*pd_mInBuf);

  outBuf.put_char_array((CORBA::Char*) &d,8);
}
#endif



void tcParseEngine::parseAny(MemBufferedStream& outBuf)
{
  CORBA::Any a;
  if (pd_bufType) a <<= (*pd_nInBuf);
  else a <<= (*pd_mInBuf);

#if !defined(NO_FLOAT)
  if (pd_reAlign) a.NP_memAlignMarshal(outBuf);
  else 
#endif
    a >>= outBuf;
}



void tcParseEngine::parsePrincipal(MemBufferedStream& outBuf)
{
  CORBA::ULong seqLen = 0;

  if (pd_bufType)
    {
      seqLen <<= (*pd_nInBuf);
      if ((*pd_nInBuf).overrun(seqLen)) 
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }
  else
    {
      seqLen <<= (*pd_mInBuf);
      if ((*pd_mInBuf).overrun(seqLen)) 
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }

  seqLen >>= outBuf;

  CORBA::Char* a = 0;
  a = new CORBA::Char[seqLen];
  if (!a) throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);

  try
    {
      if (pd_bufType) (*pd_nInBuf).get_char_array(a,seqLen);
      else (*pd_mInBuf).get_char_array(a,seqLen);

      outBuf.put_char_array(a,seqLen);
    }
  catch(...)
    {
      delete[] a;
      throw;
    }
  
  delete[] a;
}



void tcParseEngine::parseObjectRef(MemBufferedStream& outBuf)
{
  CORBA::Boolean bOrder = skipInitialStrings();

  CORBA::ULong idlen = 0;
  CORBA::Char* id = 0;
  IOP::TaggedProfileList* profiles = 0;

  try 
    {
      if (pd_bufType) idlen <<= (*pd_nInBuf);
      else idlen <<= (*pd_mInBuf);
      
      if (idlen <= 1)
	{
	  CORBA::Char _id;
		
	  if (!idlen)
	    {
#ifdef NO_SLOPPY_NIL_REFERENCE
	      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
#else
	      idlen = 1;
	      _id = '\0';
#endif
	    }
	  else
	    {
	      if (pd_bufType) _id <<= (*pd_nInBuf);
	      else _id <<= (*pd_mInBuf);
	      if ((char)_id != '\0')
		throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
	    }
		
	  idlen >>= outBuf;
	  _id >>= outBuf;
	  unsetByteOrder(bOrder);
	  return;
	}
      else if (pd_bufType)
	{
	  if ((*pd_nInBuf).overrun(idlen))
	    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
	}
      else
	{
	  if ((*pd_mInBuf).overrun(idlen))
	    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
	}   

      idlen >>= outBuf;
      id = new CORBA::Char[idlen];
      if (!id) throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);      

      if (pd_bufType) (*pd_nInBuf).get_char_array(id,idlen);
      else (*pd_mInBuf).get_char_array(id,idlen);
      if (id[idlen-1] != '\0') throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
      outBuf.put_char_array(id,idlen);

      profiles = new IOP::TaggedProfileList();
      if (!profiles) throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);

      if (pd_bufType) *profiles <<= (*pd_nInBuf);
      else *profiles <<= (*pd_mInBuf);

      *profiles >>= outBuf;
      delete profiles;
      profiles = 0;
      delete[] id;
      id = 0;
      unsetByteOrder(bOrder);
    }
  catch(...)
    {
      if (id) delete[] id;
      if (profiles) delete profiles;
      throw;
    }
}



void tcParseEngine::parseStructExcept(MemBufferedStream& outBuf)
{
  CORBA::Boolean bOrder = skipInitialStrings();
  pd_tmpParam.skip(0,omni::ALIGN_4);

  CORBA::ULong numMems = 0;
  numMems <<= pd_tmpParam;

  if ((numMems == 0 && pd_tck == CORBA::tk_struct) || 
      numMems*sizeof(CORBA::ULong) > pd_tmpParam.unRead())
    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);

  for(CORBA::ULong count = 0; count < numMems; count++)
    {
      pd_tmpParam.skip(0,omni::ALIGN_4);

      CORBA::ULong _len = 0;

      _len <<= pd_tmpParam;

      if (_len == 0 || _len > pd_tmpParam.unRead()) 
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
	    
      pd_tmpParam.skip(_len);
      pd_tmpParam.skip(0,omni::ALIGN_4);
	    
      CORBA::ULong member_tck = 0;
      member_tck <<= pd_tmpParam;
      setTCKind(member_tck);
      parseTC(outBuf);
    }
  unsetByteOrder(bOrder);
}


void tcParseEngine::parseUnion(MemBufferedStream& outBuf)
{
  CORBA::Boolean bOrder = skipInitialStrings();

  // pd_tmpParam contains complex parameter list of typecode union.
  // The content of the parameter list is defined in CORBA spec. table 12-2.
  //
  // Skip repository ID and union name in pd_tmpParam
  pd_tmpParam.skip(0,omni::ALIGN_4);
  CORBA::TypeCode unExpandedTC(CORBA::tk_null);

  // Extract typecode of the discriminant from pd_tmpParam
  unExpandedTC <<= pd_tmpParam;

  CORBA::TypeCode_var discrimTCp = unExpandedTC.NP_aliasExpand();
	
  CORBA::ULong discrimLen = 0;  
  CORBA::Char* discrim = 0;

  if (pd_bufType) 
    discrim = tcParseEngine::getUnionLabel(discrimTCp->pd_tck,(*pd_nInBuf),
					   discrimLen);
  else 
    discrim = tcParseEngine::getUnionLabel(discrimTCp->pd_tck,(*pd_mInBuf),
					   discrimLen);

  switch(discrimLen)
    {
    case 1:
      {
	outBuf.put_char_array(discrim,1);
	break;
      }
    case 2:
      {
	CORBA::UShort* s1 = (CORBA::UShort*) ((void*) discrim);
	*s1 >>= outBuf;
	break;
      }
    case 4:
      {
	CORBA::ULong* l1 = (CORBA::ULong*) ((void*) discrim);
	*l1 >>= outBuf;
	break;
      }
    default:
      break;
    }

  CORBA::Long def = 0;
  CORBA::ULong numMems = 0;

  try
    {
      // Extract (default used) and (count) field from pd_tmpParam
      def <<= pd_tmpParam;
      if (def > 0 &&  (4+(def*(1+2*sizeof(CORBA::ULong))) > 
		       pd_tmpParam.unRead()))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);

      numMems <<= pd_tmpParam;
      if (numMems == 0 || numMems*(1+(2*sizeof(CORBA::ULong))) > 
	  pd_tmpParam.unRead()) 
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
    }
  catch(...)
    {
      delete[] discrim;
      throw;
    }

  CORBA::ULong finalTCK,finalTCpos,defTCK,defTCpos;
  finalTCpos = defTCpos = finalTCK = defTCK = 0;

  for (CORBA::ULong count = 0; count < numMems; count++)
    {
      if (discrimLen == 2) pd_tmpParam.skip(0,omni::ALIGN_2);
      else if (discrimLen == 4) pd_tmpParam.skip(0,omni::ALIGN_4);

      CORBA::ULong discrimMemberLen = 0;
      CORBA::Char* discrimMember =
	tcParseEngine::getUnionLabel(discrimTCp->pd_tck,pd_tmpParam,
				                       discrimMemberLen);

      CORBA::ULong _len = 0;

      try
	{
	  _len <<= pd_tmpParam;
	  if (_len == 0 || _len > pd_tmpParam.unRead()) 
	    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
	  pd_tmpParam.skip(_len);
	  
	  CORBA::ULong tmpTCpos = pd_tmpParam.alreadyRead();
	  CORBA::TypeCode_var tmpTC = new CORBA::TypeCode(CORBA::tk_null);
	  *tmpTC <<= pd_tmpParam;
	  
	  if (count == (CORBA::ULong) def)
	    {
	      defTCK = (CORBA::ULong) tmpTC->kind();	
	      defTCpos = omni::align_to(tmpTCpos,omni::ALIGN_4);
	      defTCpos += 4;		
	    }
	  else if (memcmp((void*) discrim,(void*) discrimMember, 
			                               discrimLen) == 0)
	    {
	      finalTCK = (CORBA::ULong) tmpTC->kind();	
	      finalTCpos = omni::align_to(tmpTCpos,omni::ALIGN_4);
	      finalTCpos += 4;
	    }
	    
	  delete[] discrimMember;
	  discrimMember = 0;
	}
      catch(...)
	{
	  if (discrimMember) delete[] discrimMember;
	  delete[] discrim;
	  throw;
	}
    }

  delete[] discrim;

  CORBA::ULong orig_tck = (CORBA::ULong) pd_tck;
  CORBA::ULong origPos = (CORBA::ULong) pd_tmpParam.alreadyRead();

  if (finalTCpos > 0)
    {
      setTCKind(finalTCK); setTCPos(finalTCpos);
      parseTC(outBuf);
    }
  else if(def >= 0 && defTCpos > 0) 
    {
      setTCKind(defTCK); setTCPos(defTCpos);
      parseTC(outBuf);
    }
  else 
    {
      // The value of the discriminant does not correspond to any union
      // branch and there is no default branch in the union.
      // In this case, the union is said to contain an implicit default.
      // Its value is composed solely of the discriminator value.
      // Do nothing here as there is nothing to extract from the input.
    }
	
  setTCKind(orig_tck); setTCPos(origPos);
  unsetByteOrder(bOrder);
}



void tcParseEngine::parseEnum(MemBufferedStream& outBuf)
{
  CORBA::Boolean bOrder = skipInitialStrings();
  pd_tmpParam.skip(0,omni::ALIGN_4);

  CORBA::ULong numMems = 0;
  numMems <<= pd_tmpParam;

  if (numMems == 0 || numMems*(sizeof(CORBA::ULong)+1) > pd_tmpParam.unRead())
    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
	

  for (CORBA::ULong count = 0; count < numMems; count++)
    {
      pd_tmpParam.skip(0,omni::ALIGN_4);
	
      CORBA::ULong _len;
      _len <<= pd_tmpParam;
      if (_len == 0 || _len > pd_tmpParam.unRead())
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      pd_tmpParam.skip(_len);
    }

  CORBA::ULong member;
  if (pd_bufType) member <<= (*pd_nInBuf);
  else member <<= (*pd_mInBuf);

  member >>= outBuf;

  unsetByteOrder(bOrder);
}



void tcParseEngine::parseString(MemBufferedStream& outBuf)
{
  CORBA::ULong strLen = 0;

  if (pd_bufType)
    {
      strLen <<= (*pd_nInBuf);
      if (strLen == 0 || (*pd_nInBuf).overrun(strLen)) 
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }
  else
    {
      strLen <<= (*pd_mInBuf);
      if (strLen == 0 || (*pd_mInBuf).overrun(strLen)) 
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }

  strLen >>= outBuf;
	
  CORBA::Char* a = 0;
  a = new CORBA::Char[strLen];
  if (!a) throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);

  try
    {
      if (pd_bufType) (*pd_nInBuf).get_char_array(a, strLen);
      else (*pd_mInBuf).get_char_array(a, strLen);

      outBuf.put_char_array(a, strLen);
    }
  catch(...)
    {
      delete[] a;
      throw;
    }

  delete[] a;
  if (pd_tmpParam.alreadyRead() > 0) pd_tmpParam.skip(sizeof(CORBA::ULong));
}



void tcParseEngine::parseSeqArray(MemBufferedStream& outBuf)
{
  if (pd_tmpParam.alreadyRead() > 0) 
    {
      if (pd_tmpParam.overrun(4))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      pd_tmpParam.skip(4);
    }
  CORBA::Boolean bOrder = setByteOrder();
  
  CORBA::ULong numMems = 0;
		
  if (pd_tck == CORBA::tk_array)
    {
      CORBA::ULong currPos = pd_tmpParam.alreadyRead();
      CORBA::TypeCode tmpTC(CORBA::tk_null);
      tmpTC <<= pd_tmpParam;
      numMems <<= pd_tmpParam;
      setTCPos(currPos);
    }
  else 
    {
      if (pd_bufType) numMems <<= (*pd_nInBuf);
      else numMems <<= (*pd_mInBuf);

      numMems >>= outBuf;
    }
		
  if (numMems > 0)
    {
      CORBA::ULong member_tck;
      member_tck <<= pd_tmpParam;
      setTCKind(member_tck);

      if ( pd_tck == (CORBA::ULong) CORBA::tk_char || 
	   pd_tck == (CORBA::ULong) CORBA::tk_octet ||
	   pd_tck == (CORBA::ULong) CORBA::tk_boolean )
	{
	  // Optimization - no need for byte swapping

	  CORBA::Char* a = 0;

	  try
	    {
	      if (pd_bufType)
		{
		  if ((*pd_nInBuf).overrun(numMems)) 
		    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
	  
		  a = new CORBA::Char[numMems];
		  if (!a) throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
		
		  (*pd_nInBuf).get_char_array(a,numMems);
		}
	      else
		{
		  if ((*pd_mInBuf).overrun(numMems)) 
		    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
	  
		  a = new CORBA::Char[numMems];
		  if (!a) throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
		
		  (*pd_mInBuf).get_char_array(a,numMems);
		}

	      outBuf.put_char_array(a,numMems);
	      delete[] a;
	      a = 0;
	    }
	  catch (...)
	    {
	      if (a) delete[] a;
	      throw;
	    }
	}
      else
	{
	  CORBA::ULong origPos = pd_tmpParam.alreadyRead();
	  for (CORBA::ULong count=0; count < numMems; count++)
	    {
	      setTCKind(member_tck); setTCPos(origPos);
	      parseTC(outBuf);
	    }
	}
    }
  else
    {
      // Zero-length sequence - just unmarshal the typecode.
      CORBA::TypeCode tempTC(CORBA::tk_null);
      tempTC <<= pd_tmpParam;
    }

  pd_tmpParam.skip(0,omni::ALIGN_4);
  pd_tmpParam.skip(4);
  unsetByteOrder(bOrder);
}



void tcParseEngine::parseAlias(MemBufferedStream& outBuf)
{
  CORBA::Boolean bOrder = skipInitialStrings();
  pd_tmpParam.skip(0,omni::ALIGN_4);

  CORBA::ULong member_tck = 0;
  member_tck <<= pd_tmpParam;
  setTCKind(member_tck);
  parseTC(outBuf);
  unsetByteOrder(bOrder);
}



/**************************************************************************/



CORBA::Boolean tcParseEngine::skipInitialStrings()
{
  // Skip Repository ID and Name Strings at the beginning of typecodes.

  if (pd_tmpParam.alreadyRead() > 0)
    {
      if (pd_tmpParam.unRead() < 4)
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      pd_tmpParam.skip(4);
    }
  CORBA::Boolean bOrder = setByteOrder();

  CORBA::ULong _len = 0;
  
  _len <<= pd_tmpParam;
  if (_len == 0 || _len > pd_tmpParam.unRead()) 
    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
  pd_tmpParam.skip(_len);
  pd_tmpParam.skip(0,omni::ALIGN_4);

  _len <<= pd_tmpParam;
  if (_len == 0 || _len > pd_tmpParam.unRead()) 
    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
  pd_tmpParam.skip(_len);

  return bOrder;
}


CORBA::Boolean tcParseEngine::setByteOrder()
{
  if (pd_tmpParam.unRead() < 4)
    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);

  CORBA::Boolean origOrder = pd_tmpParam.byteOrder();
  CORBA::Char tmpOrder = 0;
  tmpOrder <<= pd_tmpParam;
  
  if (tmpOrder > 1)
    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);

  CORBA::Boolean bOrder = tmpOrder;
  if (bOrder != origOrder) pd_tmpParam.byteOrder(bOrder);

  pd_tmpParam.skip(3);

  return origOrder;
}

