// -*- Mode: C++; -*-
//                            Package   : omniORB2
// typecode.cc                Created on: 23/06/97
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
//      Implementation of the CORBA::TypeCode psuedo object
//	


/* 
 * $Log$
 * Revision 1.8  1998/04/18 10:11:17  sll
 * Corrected signature of one TypeCode ctor.
 *
 * Revision 1.7  1998/04/08 16:07:50  sll
 * Minor change to help some compiler to find the right TypeCode ctor.
 *
 * Revision 1.6  1998/04/08 14:07:26  sll
 * Added workaround in CORBA::TypeCode::_nil() for a bug in DEC Cxx v5.5.
 *
 * Revision 1.5  1998/04/07 19:40:53  sll
 * Moved inline member functions to this module.
 *
 * Revision 1.4  1998/03/17 12:52:19  sll
 * Corrected typo.
 *
 * Revision 1.3  1998/03/17 12:12:31  ewc
 * Bug fix to NP_aliasExpand()
 *
// Revision 1.2  1998/02/20  14:45:43  ewc
// Changed to compile with aCC on HPUX
//
// Revision 1.1  1998/01/27  15:41:24  ewc
// Initial revision
//
 */


#include <omniORB2/CORBA.h>
#include "tcParseEngine.h"

static CORBA::TypeCode_ptr        _nil_TypeCodeV = 0;
static CORBA::IDLType_ptr         _nil_IDLTypeV = 0;

#ifndef HAS_Cplusplus_Namespace
// deprecated. Left here for backward compatability
static CORBA::_nil_TypeCode __nil_TypeCode__;
const CORBA::TypeCode_ptr CORBA::__nil_TypeCode = &__nil_TypeCode__;
CORBA::IDLType_ptr        CORBA::__nil_IDLType;
#endif

CORBA::TypeCode::TypeCode(CORBA::TCKind t, CORBA::ULong maxLen)
{
  // Note that the use of maxLen in this constructor is forced by MSVC++ 4.2
  // as it can't cope with an in-place constructor with just an enum as an
  // argument.

  pd_tck = t;
  if (pd_tck == CORBA::tk_string) pd_maxLen = maxLen;
  else maxLen = 0;
} 


CORBA::TypeCode::TypeCode(const char* repoId, const char* name)
{
  pd_tck = CORBA::tk_objref;
  pd_maxLen = 0;

  omni::myByteOrder >>= pd_param;

  CORBA::ULong _len = strlen( (const char*) repoId) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) repoId),_len);

  _len = strlen ( (const char*) name) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) name),_len);

}

CORBA::TypeCode::TypeCode(const char* repoId, const char* name, 
			  const CORBA::TypeCode& contentTC)
{
  pd_tck = CORBA::tk_alias;
  pd_maxLen = 0;

  omni::myByteOrder >>= pd_param;

  CORBA::ULong _len = strlen( (const char*) repoId) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) repoId),_len);

  _len = strlen ( (const char*) name) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) name),_len);

  contentTC >>= pd_param;
}
  
  

CORBA::TypeCode::TypeCode(CORBA::TCKind t, CORBA::ULong length, 
			  const CORBA::TypeCode& contentTC)
{
  if (t != CORBA::tk_sequence && t != CORBA::tk_array) 
    {
      throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
    }

  pd_tck = t;
  pd_maxLen = 0;

  omni::myByteOrder >>= pd_param;
  contentTC >>= pd_param;
  length >>= pd_param;
}
  

CORBA::TypeCode::TypeCode(CORBA::TCKind t, const char* repoId,
			  const char* name,const CORBA::StructMemberSeq& members)
{
  if (t != CORBA::tk_struct && t != CORBA::tk_except)
    {
      throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
    }

  pd_tck = t;
  pd_maxLen = 0;
  
  omni::myByteOrder >>= pd_param;
  
  CORBA::ULong _len = strlen( (const char*) repoId) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) repoId),_len);

  _len = strlen ( (const char*) name) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) name),_len);

  CORBA::ULong count = members.length();
  count >>= pd_param;

  for (CORBA::ULong i = 0; i < count; i++)
    {
      _len = strlen((const char*) (members[i]).name) + 1;
      _len >>= pd_param;
      pd_param.put_char_array((const CORBA::Char*) ((const char*) 
						      (members[i]).name),_len);
      *(members[i]).type >>= pd_param;
    }
}


CORBA::TypeCode::TypeCode(const char* repoId, const char* name, 
			                     const EnumMemberSeq& members)
{
  pd_tck = tk_enum;
  pd_maxLen = 0;
  
  omni::myByteOrder >>= pd_param;
  
  CORBA::ULong _len = strlen( (const char*) repoId) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) repoId),_len);

  _len = strlen ( (const char*) name) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) name),_len);

  CORBA::ULong count = members.length();
  count >>= pd_param;

  for (CORBA::ULong i = 0; i < count; i++)
    {
      _len = strlen((const char*) members[i]) + 1;
      _len >>= pd_param;
      pd_param.put_char_array((const CORBA::Char*) ((const char*) members[i]),
			         _len);
    }
}

CORBA::TypeCode::TypeCode(const char* repoId, const char* name, 
		   const CORBA::TypeCode_ptr discriminatorTC, 
		   const UnionMemberSeq& members)
{
  pd_tck = CORBA::tk_union;
  pd_maxLen = 0;
  omni::myByteOrder >>= pd_param;

  CORBA::ULong _len = strlen( (const char*) repoId) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) repoId),_len);

  _len = strlen ( (const char*) name) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) name),_len);

  *discriminatorTC >>= pd_param;

  CORBA::ULong count = members.length();
  CORBA::Long def = -1;
  CORBA::ULong i = 0;

  for (i = 0; i < count; i++)
    {
        CORBA::TypeCode_var defaultTCp = (members[i].label).type();
	if (defaultTCp->kind() == CORBA::tk_octet)
	  {
	    def = (CORBA::Long) i;
	    break;
	  }
    }
 
  def >>= pd_param;
  count >>= pd_param;

  CORBA::TypeCode_var discrim = discriminatorTC->NP_aliasExpand();

  for (i = 0; i < count; i++)
    {
      CORBA::TypeCode_var valTC = (members[i]).label.type();
      
      if (discriminatorTC->equal(valTC) || i == (CORBA::ULong) def)
	{
	  MemBufferedStream tmp_mbuf;

	  (members[i]).label.NP_getBuffer(tmp_mbuf);	
  
	  switch(discrim->kind())
	    {
	    case CORBA::tk_char:
	    case CORBA::tk_boolean:
	      {
		CORBA::Char a;
		if (i == (CORBA::ULong) def) a = 0;
		else a <<= tmp_mbuf;
		a >>= pd_param;
		break;
	      }
		
	    case CORBA::tk_short:
	    case CORBA::tk_ushort:
	      {
		CORBA::UShort a;
		if (i == (CORBA::ULong) def) a = 0;
		else a <<= tmp_mbuf;
		a >>= pd_param;
		break;
	      }

	    case CORBA::tk_long:
	    case CORBA::tk_ulong:
	    case CORBA::tk_enum:
	      {
		CORBA::ULong a;
		if (i == (CORBA::ULong) def) a = 0;
		else a <<= tmp_mbuf;
		a >>= pd_param;
		break;
	      }
	      
	    default:
	      {
		throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
	      }
	    }
	}
      else throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

      _len = strlen((const char*) (members[i]).name) + 1;
      _len >>= pd_param;
      pd_param.put_char_array((const CORBA::Char*) ((const char*) 
						      (members[i]).name),_len);
      *((members[i]).type) >>= pd_param;      
    }
}
      

/**************************************************************************/

// Constructors for stubs.

CORBA::TypeCode::TypeCode(CORBA::TCKind t, const char* repoId, 
			  const char* name, 
			  CORBA::PR_structMember* members, 
			  CORBA::ULong num_mems)
{
  if (t != CORBA::tk_struct && t != CORBA::tk_except)
    {
      throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
    }

  pd_tck = t;
  pd_maxLen = 0;
  
  omni::myByteOrder >>= pd_param;
  
  CORBA::ULong _len = strlen( (const char*) repoId) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) repoId),_len);

  _len = strlen ( (const char*) name) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) name),_len);

  CORBA::ULong count = num_mems;
  count >>= pd_param;

  for (CORBA::ULong i = 0; i < count; i++)
    {
      _len = strlen((const char*) (members[i]).name) + 1;
      _len >>= pd_param;
      pd_param.put_char_array((const CORBA::Char*) ((const char*) 
						      (members[i]).name),_len);
      *(members[i]).type >>= pd_param;
      CORBA::release((members[i]).type);
    }
}
    
CORBA::TypeCode::TypeCode(const char* repoId, const char* name, 
			  char** members, CORBA::ULong num_mems)   
{
  pd_tck = tk_enum;
  pd_maxLen = 0;
  
  omni::myByteOrder >>= pd_param;
  
  CORBA::ULong _len = strlen( (const char*) repoId) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) repoId),_len);

  _len = strlen ( (const char*) name) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) name),_len);

  CORBA::ULong count = num_mems;
  count >>= pd_param;

  for (CORBA::ULong i = 0; i < count; i++)
    {
      _len = strlen((const char*) members[i]) + 1;
      _len >>= pd_param;
      pd_param.put_char_array((const CORBA::Char*) ((const char*) members[i]),
			         _len);
    }
}

CORBA::TypeCode::TypeCode(const char* repoId, const char* name, 
			  const CORBA::TypeCode& discriminatorTC, 
			  CORBA::PR_unionMember* members, 
			  CORBA::ULong num_mems, CORBA::Long def)
{  
  pd_tck = CORBA::tk_union;
  pd_maxLen = 0;
  omni::myByteOrder >>= pd_param;

  CORBA::ULong _len = strlen( (const char*) repoId) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) repoId),_len);

  _len = strlen ( (const char*) name) + 1;
  _len >>= pd_param;
  pd_param.put_char_array((const CORBA::Char*) ((const char*) name),_len);

  discriminatorTC >>= pd_param;

  CORBA::ULong count = num_mems;
  CORBA::ULong i = 0;

  if (def >= (CORBA::Long) count && def >= 0) 
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  def >>= pd_param;
  count >>= pd_param;

  CORBA::TypeCode_var discrim = discriminatorTC.NP_aliasExpand();

  for (i = 0; i < count; i++)
    {
      switch(discrim->kind())
	{
	case CORBA::tk_char:
	case CORBA::tk_boolean:
	  {
	    CORBA::Char a;
	    if (i == (CORBA::ULong) def && def >= 0) a = 0;
	    else a = (CORBA::Char) (members[i]).label;
	    
	    a >>= pd_param;
	    break;
	  }
	  
	case CORBA::tk_short:
	case CORBA::tk_ushort:
	  {
	    CORBA::UShort a;
	    if (i == (CORBA::ULong) def && def >= 0) a = 0;
	    else a = (CORBA::UShort) (members[i]).label;
	    a >>= pd_param;
	    break;
	  }
	  
	case CORBA::tk_long:
	case CORBA::tk_ulong:
	case CORBA::tk_enum:
	  {
	    CORBA::ULong a;
	    if (i == (CORBA::ULong) def && def >= 0) a = 0;
	    else a = (members[i]).label;
	    a >>= pd_param;
	    break;
	  }
	      
	default:
	  {
	    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
	  }
	}

      _len = strlen((const char*) (members[i]).name) + 1;
      _len >>= pd_param;
      pd_param.put_char_array((const CORBA::Char*) ((const char*) 
						    (members[i]).name),_len);
      *((members[i]).type) >>= pd_param;      
      CORBA::release((members[i]).type);
    }
}


/**************************************************************************/

void 
CORBA::TypeCode::operator>>= (NetBufferedStream& s) const
{
  if (NP_is_nil()) throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
                      
  ::operator>>=((CORBA::ULong) pd_tck,s);   // Encode the typecode itself


  if (pd_tck == CORBA::tk_string)
    {
      // Encode the maximum length parameter.
      ::operator>>=(pd_maxLen,s);
    }
  else if (pd_tck > CORBA::tk_Principal)
    {
      ::operator>>=((CORBA::ULong) pd_param.alreadyWritten(),s); 
                                                   // Encode MemBuffStream size
      
	s.put_char_array((_CORBA_Char*) pd_param.data(),
			                         pd_param.alreadyWritten());
	                                           // Encode MemBuffStream
    }
}



void
CORBA::TypeCode::operator<<= (NetBufferedStream& s)
{
  CORBA::ULong tcExtr;
  

  tcExtr <<= s;           // Extract typecode
  if (tcExtr > (CORBA::ULong) CORBA::tk_except) 
    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);  // Not a valid typecode

  pd_tck = (CORBA::TCKind) tcExtr;
  pd_param.rewind_inout_mkr();
  if (pd_tck == CORBA::tk_string) 
    {
      pd_maxLen <<= s;    // Extract maximum length parameter
    }
  else if(pd_tck > CORBA::tk_Principal)
    {
      CORBA::ULong sz;
      sz <<= s;

      if (sz > s.RdMessageUnRead()) _CORBA_marshal_error();

      void* p = pd_param.setSize((size_t) sz);      
      s.get_char_array((_CORBA_Char*) p, (int) sz);      
      if (((_CORBA_Char*)p)[0] > (_CORBA_Char) 1) 
      	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
      else pd_param.byteOrder((_CORBA_Boolean)((_CORBA_Char*)p)[0]);
    }
}




void 
CORBA::TypeCode::operator>>= (MemBufferedStream& s) const
{
  if (NP_is_nil()) throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);

  ::operator>>=((CORBA::ULong) pd_tck,s);   // First, encode the typecode 
                                            // itself


  if (pd_tck == CORBA::tk_string)
    {
      ::operator>>=(pd_maxLen,s);
    }
  else if (pd_tck > CORBA::tk_Principal)
    {
      ::operator>>=((CORBA::ULong) pd_param.alreadyWritten(),s); 
                                                // Encode MemBuffStream size
      
	s.put_char_array((_CORBA_Char*) pd_param.data(),
			                         pd_param.alreadyWritten());
	                                        // Encode MemBuffStream
    }
}



void
CORBA::TypeCode::operator<<= (MemBufferedStream& s)
{
  CORBA::ULong tcExtr;
  
  tcExtr <<= s;           // Extract typecode
  if (tcExtr > (CORBA::ULong) CORBA::tk_except)
    {
      if (tcExtr == (CORBA::ULong) 0xffffffff && omniORB::traceLevel >= 10) {
	omniORB::log << "\nIndirection TypeCodes not yet handled.\n";
	omniORB::log.flush();
      }
      throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
    }


  pd_tck = (CORBA::TCKind) tcExtr;
  pd_param.rewind_inout_mkr();

  if (pd_tck == CORBA::tk_string) 
    {
      pd_maxLen <<= s;    // Extract maximum length parameter
    }
  else if(pd_tck > CORBA::tk_Principal)
    {
      CORBA::ULong sz;
      sz <<= s;

      if (s.overrun(sz)) _CORBA_marshal_error(); 
      void* p = pd_param.setSize((size_t) sz);      
      s.get_char_array((_CORBA_Char*) p, (int) sz);
      if (((_CORBA_Char*)p)[0] > (_CORBA_Char) 1) 
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
      else pd_param.byteOrder((_CORBA_Boolean) (((_CORBA_Char*)p)[0]));
    }
}
 


size_t
CORBA::TypeCode::NP_alignedSize(size_t _initialoffset) const
{

  CORBA::ULong _msgsize = _initialoffset;              // Size of TypeCode.
  _msgsize = omni::align_to(_msgsize,omni::ALIGN_4);
  _msgsize += 4;

  if (pd_tck > CORBA::tk_Principal)
    {
      // tk_string max. length / MemBufferedStream length.
      _msgsize = omni::align_to(_msgsize,omni::ALIGN_4);
      _msgsize += 4;

      if (pd_tck != CORBA::tk_string) _msgsize += 
				     (CORBA::ULong) pd_param.alreadyWritten();
    }


  return _msgsize;
}
      

CORBA::Boolean
CORBA::TypeCode::NP_expandEqual(CORBA::TypeCode_ptr TCp,
				CORBA::Boolean expand) const
{
  if (!expand || (pd_tck != CORBA::tk_alias && TCp->pd_tck != CORBA::tk_alias))
    {
      return PR_equal(TCp,expand);
    }
  else if(pd_tck == CORBA::tk_alias && TCp->pd_tck == CORBA::tk_alias)
    {
      CORBA::TypeCode_var thisContent = this->content_type();
      CORBA::TypeCode_var TCpContent = TCp->content_type();
      
      return thisContent->NP_expandEqual(TCpContent,expand);
    }
  else if (this->pd_tck == CORBA::tk_alias)
    {
      CORBA::TypeCode_var thisContent = this->content_type();
      
      return thisContent->NP_expandEqual(TCp,expand);
	    } 
  else if (TCp->pd_tck == CORBA::tk_alias)
    {
      CORBA::TypeCode_var TCpContent = TCp->content_type();
      
      return this->NP_expandEqual(TCpContent,expand);
    }
  else return 0;
}

void 
CORBA::TypeCode::NP_fillInit(MemBufferedStream& mbuf) const
{
  CORBA::ULong fillerLen;

  if (pd_tck < CORBA::tk_objref || pd_tck == CORBA::tk_string) return;
  else if (omniORB::tcAliasExpand)
    {
      CORBA::TypeCode_var member_tc = this->NP_completeExpand();
      if (member_tc->kind() < CORBA::tk_objref || 
	  member_tc->kind() == CORBA::tk_string) return;

      fillerLen = (member_tc->pd_param).alreadyWritten() % 4;
    }
  else if (pd_tck == CORBA::tk_alias || pd_tck == CORBA::tk_sequence) 
    {
      CORBA::TypeCode_var member_tc = this->NP_aliasExpand();
      if (member_tc->kind() < CORBA::tk_objref || 
	  member_tc->kind() == CORBA::tk_string) return;

      fillerLen = (member_tc->pd_param).alreadyWritten() % 4;
    }
  else fillerLen = pd_param.alreadyWritten() % 4;

  
  if (fillerLen > 0)
    {
      CORBA::Char* filler = new Char[fillerLen];
      try
	{
	  mbuf.put_char_array(filler,fillerLen);
	}
      catch(...)
	{
	  delete[] filler;
	  throw;
	}
      
      delete[] filler;
    }  
}


CORBA::TypeCode_ptr 
CORBA::TypeCode::NP_aliasExpand() const
{
  // Expand aliases of a top-level TypeCode
  if (pd_tck == CORBA::tk_alias)
    {
      CORBA::TypeCode_var member_tc = content_type();
      return member_tc->NP_aliasExpand();
    }
  else if (pd_tck == CORBA::tk_sequence)
    {
      // Remove aliases from a sequence
      CORBA::TypeCode_var member_tc = content_type();
      if (member_tc->kind() == CORBA::tk_alias || member_tc->kind() ==
	  CORBA::tk_sequence)
	{
	  CORBA::TypeCode_var new_tc = member_tc->NP_aliasExpand();
	  return new CORBA::TypeCode(CORBA::tk_sequence,this->length(),
				     *(new_tc.operator->()));
	}
    }
  
  return CORBA::TypeCode::_duplicate((CORBA::TypeCode_ptr) this);
}


CORBA::TypeCode_ptr
CORBA::TypeCode::NP_completeExpand() const
{
  // Completely expand all aliases of a TypeCode 
  // (including aliases of member TypeCodes) 

  if (pd_tck <= CORBA::tk_objref || pd_tck == CORBA::tk_string || 
      pd_tck == CORBA::tk_enum)
    {
      return CORBA::TypeCode::_duplicate((CORBA::TypeCode_ptr) this);
    }
  else if (pd_tck == CORBA::tk_alias)
    {
      CORBA::TypeCode_var member_tc = this->NP_aliasExpand();
      return member_tc->NP_completeExpand();
    }

  CORBA::TypeCode_ptr tc = CORBA::TypeCode::_nil();

  switch(pd_tck)
    {
    case CORBA::tk_struct:
    case CORBA::tk_except:
      {
	CORBA::ULong memberCount = this->member_count();
	CORBA::ULong count = 0;
	CORBA::StructMemberSeq memberSeq;
	memberSeq.length(memberCount);
	for (count = 0; count < memberCount; count++)
	  {
	    CORBA::TypeCode_var memberTC = this->member_type(count);
	    (memberSeq[count]).name = (const char*) this->member_name(count);
	    (memberSeq[count]).type = memberTC->NP_completeExpand();
	    (memberSeq[count]).type_def = (CORBA::IDLType_ptr) 0;
	  }
	
	tc = new CORBA::TypeCode(pd_tck,this->id(),this->name(),memberSeq);
	break;
      }

    case CORBA::tk_union:
      {
	CORBA::ULong memberCount = this->member_count();
	CORBA::ULong count = 0;
	CORBA::UnionMemberSeq memberSeq;
	memberSeq.length(memberCount);
	for (count = 0; count < memberCount; count++)
	  {
	    (memberSeq[count]).name = (const char*) this->member_name(count);
	   
	    CORBA::Any* memberLabel = this->member_label(count);
	    
	    (memberSeq[count]).label = *memberLabel;
	    delete memberLabel;

	    CORBA::TypeCode_var memberTC = this->member_type(count);
	    (memberSeq[count]).type = memberTC->NP_completeExpand();
	    (memberSeq[count]).type_def = (CORBA::IDLType_ptr) 0;
	  }

	CORBA::TypeCode_ptr discrimTC = this->discriminator_type();
	try
	  {
	    tc = new CORBA::TypeCode(this->id(),this->name(),
				     discrimTC,memberSeq);
	  }
	catch(...)
	  {
	    CORBA::release(discrimTC);
	    throw;
	  }

	CORBA::release(discrimTC);
	break;
      }

    case CORBA::tk_sequence:
    case CORBA::tk_array:
      {
	CORBA::TypeCode_var contentTC = this->content_type();
	CORBA::TypeCode_ptr expandedContentTC = contentTC->NP_completeExpand();
	
	try
	  {
	    tc = new CORBA::TypeCode(pd_tck,this->length(),*expandedContentTC);
	  }
	catch(...)
	  {
	    CORBA::release(expandedContentTC);
	    throw;
	  }

	CORBA::release(expandedContentTC);
	break;
      }

    default:
      {
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
	break;
      }
    }

  return tc;
}

/**************************************************************************/

CORBA::TypeCode::TypeCode(const CORBA::TypeCode& tc)
{
  pd_tck =    tc.pd_tck;
  pd_maxLen = tc.pd_maxLen;

  
  if (tc.pd_tck > CORBA::tk_Principal && tc.pd_tck != CORBA::tk_string) 
    pd_param = tc.pd_param;
}



CORBA::TypeCode& 
CORBA::TypeCode::operator=(const CORBA::TypeCode& tc)     
{
  if (this != &tc) 
    {
      pd_tck    = tc.pd_tck;
      pd_maxLen = tc.pd_maxLen;
      
      if (tc.pd_tck > CORBA::tk_Principal && tc.pd_tck != CORBA::tk_string) 
	pd_param = tc.pd_param;
      else pd_param.reset();
    }

  return *this;
}


/**************************************************************************/


CORBA::Boolean
CORBA::TypeCode::PR_equal(CORBA::TypeCode_ptr TCp, CORBA::Boolean expand) const
{
  if (pd_tck != TCp->pd_tck) return 0;
  else if (pd_tck < CORBA::tk_objref) return 1; 
  else if (pd_tck == CORBA::tk_string)
    {
      if (pd_maxLen == TCp->pd_maxLen) return 1;
      else return 0;
    }
  else
    {
      MemBufferedStream myBuf(pd_param,1);
      MemBufferedStream TCpBuf(TCp->pd_param,1);

      CORBA::ULong myUL = 0;
      CORBA::ULong otUL = 0;
      
      if (pd_tck == CORBA::tk_sequence || pd_tck == CORBA::tk_array)
	{
	  myBuf.skip(1); TCpBuf.skip(1);

	  CORBA::TypeCode a(CORBA::tk_null,0),b(CORBA::tk_null,0);
	  a <<= myBuf;
	  b <<= TCpBuf;

	  if (!a.NP_expandEqual(&b,expand)) return 0;
	  
	  myUL <<= myBuf;
	  otUL <<= TCpBuf;

	  return (myUL == otUL ? 1 : 0);
	}
      else
	{
	  myBuf.skip(4);
	  TCpBuf.skip(4);
	  
	  CORBA::ULong myLen = 0;
	  CORBA::ULong TCpLen = 0;
	  myLen <<= myBuf;
	  TCpLen <<= TCpBuf;

	  if (myBuf.overrun(myLen) || TCpBuf.overrun(TCpLen)) 
	    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);

	  if (strcmp((const char*) myBuf.data(),
		         (const char*) TCpBuf.data()) != 0) 
	    {
	      if (pd_tck == CORBA::tk_objref || pd_tck == CORBA::tk_except ||
		  (myLen != 1 && TCpLen != 1)) return 0;
	    }

	  
	  myBuf.skip(myLen); myBuf.skip(0,omni::ALIGN_4);
	  TCpBuf.skip(TCpLen); TCpBuf.skip(0, omni::ALIGN_4);

	  myLen <<= myBuf;
	  TCpLen <<= TCpBuf;

	  if (myBuf.overrun(myLen) || TCpBuf.overrun(TCpLen)) 
	    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
	  
	  if (strcmp((const char*) myBuf.data(), 
		     (const char*) TCpBuf.data()) != 0 && 
	      myLen != 1 && TCpLen != 1) return 0; 
	  else if (pd_tck == CORBA::tk_objref) return 1;
	  
	  myBuf.skip(myLen);
	  TCpBuf.skip(TCpLen);

	  CORBA::TypeCode contentA(CORBA::tk_null,0), contentB(CORBA::tk_null,0);

	  CORBA::Long def = 0;

	  if (pd_tck == CORBA::tk_union || pd_tck == CORBA::tk_alias)
	    {
	      contentA <<= myBuf;
	      contentB <<= TCpBuf;

	      if (!contentA.NP_expandEqual(&contentB,expand)) return 0;
	      else if (pd_tck == CORBA::tk_alias) return 1;
	      
	      CORBA::Long TCpDef = 0;

	      def <<= myBuf;
	      TCpDef <<= TCpBuf;

	      if ((def > 0 && (4+(def*(1+2*sizeof(CORBA::ULong))) > 
			       myBuf.unRead())) || 
		  ((TCpDef > 0 && (4+(TCpDef*(1+2*sizeof(CORBA::ULong))) > 
				   TCpBuf.unRead()))))
		throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);

	      if ((def >= 0 || TCpDef >= 0) && def != TCpDef) return 0;
	    }


	  myUL <<= myBuf;
	  otUL <<= TCpBuf;
	  
	  if (myBuf.overrun(myUL*sizeof(CORBA::ULong)) || 
	      TCpBuf.overrun(otUL*sizeof(CORBA::ULong)))
	    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);

	  if(myUL != otUL) return 0;

	  // Iterate through the member list, checking for equality.
	  
	  switch (pd_tck)
	    {
	    case CORBA::tk_struct:
	    case CORBA::tk_except:
	      {
		for (CORBA::ULong i=0; i < myUL; i++)
		  {
		    myBuf.skip(0,omni::ALIGN_4);
		    TCpBuf.skip(0,omni::ALIGN_4);
		  
		    myLen <<= myBuf;
		    TCpLen <<= TCpBuf;

		    if (myBuf.overrun(myLen) || TCpBuf.overrun(TCpLen))
		      throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);

		    if (strcmp((const char*) myBuf.data(),
			          (const char*) TCpBuf.data()) != 0 &&
			myLen != 1 && TCpLen != 1) return 0;
		    
		    myBuf.skip(myLen);
		    TCpBuf.skip(TCpLen);
		    
		    CORBA::TypeCode a(CORBA::tk_null,0),b(CORBA::tk_null,0);
		    a <<= myBuf;
		    b <<= TCpBuf;
		    if (!a.NP_expandEqual(&b,expand)) return 0;
		  }
		return 1;
	      }

	    case CORBA::tk_enum:
	      {
		for (CORBA::ULong i = 0; i < myUL; i++)
		  {
		    myBuf.skip(0,omni::ALIGN_4);
		    TCpBuf.skip(0,omni::ALIGN_4);
		  
		    myLen <<= myBuf;
		    TCpLen <<= TCpBuf;

		    if (myBuf.overrun(myLen) || TCpBuf.overrun(TCpLen))
		      throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
		    
		    if (strcmp((const char*) myBuf.data(),
			          (const char*) TCpBuf.data()) != 0 &&
			myLen != 1 && TCpLen != 1) return 0;
		    
		    myBuf.skip(myLen);
		    TCpBuf.skip(TCpLen);
		  }
		return 1;
	      }  
		  

	    case CORBA::tk_union:
	      {
		CORBA::TypeCode_var contentTC = contentA.NP_aliasExpand();

		for (CORBA::ULong i = 0; i < myUL; i++)
		  {
		    switch (contentTC->pd_tck)
		      {
		      case CORBA::tk_boolean:
		      case CORBA::tk_char:
			{
			  CORBA::Char aCh, bCh;
			  aCh <<= myBuf;
			  bCh <<= TCpBuf;

			  if (aCh != bCh && i != (CORBA::ULong) def) return 0;
			  break;
			}

		      case CORBA::tk_short:
		      case CORBA::tk_ushort:
			{
			  CORBA::UShort aS, bS;
			  aS <<= myBuf;
			  bS <<= TCpBuf;

			  if (aS != bS && i != (CORBA::ULong) def) return 0;
			  break;
			}
			
		      case CORBA::tk_long:
		      case CORBA::tk_ulong:
		      case CORBA::tk_enum:
			{
			  CORBA::ULong aUL, bUL;
			  aUL <<= myBuf;
			  bUL <<= TCpBuf;

			  if (aUL != bUL && i != (CORBA::ULong) def) return 0;
			  break;
			}
			
		      default:
			throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
		      };

		    myBuf.skip(0,omni::ALIGN_4);
		    TCpBuf.skip(0,omni::ALIGN_4);
		  
		    myLen <<= myBuf;
		    TCpLen <<= TCpBuf;

		    if (myBuf.overrun(myLen) || TCpBuf.overrun(TCpLen))
		      throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
		    
		    if (strcmp((const char*) myBuf.data(),
			          (const char*) TCpBuf.data()) != 0 &&
			myLen != 1 && TCpLen != 1) return 0;
		    
		    myBuf.skip(myLen);
		    TCpBuf.skip(TCpLen);

		    CORBA::TypeCode a(CORBA::tk_null,0),b(CORBA::tk_null,0);
		    a <<= myBuf;
		    b <<= TCpBuf;
		    if (!a.NP_expandEqual(&b,expand)) return 0;    
		  }
		return 1;
	      } 

	    default:
	      {
		throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
		return 0;
	      }
	    }      
	}
    }
}


/**************************************************************************/


const char* 
CORBA::TypeCode::id() const
{
  if (pd_tck == CORBA::tk_objref || pd_tck == CORBA::tk_struct || 
      pd_tck == CORBA::tk_union || pd_tck == CORBA::tk_enum || 
      pd_tck == CORBA::tk_alias || pd_tck == CORBA::tk_except)
    {
      CORBA::ULong idLen;

      MemBufferedStream tmpBuf(pd_param,1);
      tmpBuf.skip(1);
      idLen <<= tmpBuf;
      if (idLen == 0 || tmpBuf.overrun(idLen))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);

      const char* p1 = ((const char*) tmpBuf.data());
      return p1;
    }
  else
    {
      // Operation not valid on this TypeCode.
      throw CORBA::TypeCode::BadKind();
      return 0;
    }
}



const char*
CORBA::TypeCode::name() const
{
 if (pd_tck == CORBA::tk_objref || pd_tck == CORBA::tk_struct || 
     pd_tck == CORBA::tk_union || pd_tck == CORBA::tk_enum || 
     pd_tck == CORBA::tk_alias || pd_tck == CORBA::tk_except)
    {
      CORBA::ULong idLen;

      MemBufferedStream tmpBuf(pd_param,1);
      tmpBuf.skip(1);
      idLen <<= tmpBuf;
      if (idLen == 0 || tmpBuf.overrun(idLen))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      tmpBuf.skip(idLen);

      tmpBuf.skip(0,omni::ALIGN_4);
      idLen <<= tmpBuf;
      if (idLen == 0 || tmpBuf.overrun(idLen))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);

      const char* p1 = (const char*) tmpBuf.data();
      return p1;     
    }
  else
    {
      // Operation not valid on this TypeCode.
      throw CORBA::TypeCode::BadKind();
      return 0;
    }
}



CORBA::ULong
CORBA::TypeCode::member_count() const
{
  if (pd_tck == CORBA::tk_struct || pd_tck == CORBA::tk_union || 
      pd_tck == CORBA::tk_enum || pd_tck == CORBA::tk_except)
    {
      CORBA::ULong _len;

      MemBufferedStream tmpBuf(pd_param,1);
      tmpBuf.skip(1);

      _len <<= tmpBuf;
      if (_len == 0 || tmpBuf.overrun(_len))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      tmpBuf.skip(_len);

      _len <<= tmpBuf;
      if (_len == 0 || tmpBuf.overrun(_len))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      tmpBuf.skip(_len);
      
      if (pd_tck == CORBA::tk_union)
	{
	  CORBA::TypeCode tmpTC(CORBA::tk_null,0);
	  tmpTC <<= tmpBuf;
	  tmpBuf.skip(4,omni::ALIGN_4);
	}

      CORBA::ULong count;
      count <<= tmpBuf;
      
      if ((count == 0 && pd_tck != CORBA::tk_except) || 
	   tmpBuf.overrun(count*sizeof(CORBA::ULong)))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);

      return count;
    }
  else
    {
      // Operation not valid on this TypeCode.
      throw CORBA::TypeCode::BadKind();
      return 0;
    }
}



const char* 
CORBA::TypeCode::member_name(CORBA::ULong index) const
{
 if (pd_tck == CORBA::tk_struct || pd_tck == CORBA::tk_enum || 
     pd_tck == CORBA::tk_except || pd_tck == CORBA::tk_union)
    {
      CORBA::ULong _len;

      MemBufferedStream tmpBuf(pd_param,1);
      tmpBuf.skip(1);

      _len <<= tmpBuf;
      if (_len == 0 || tmpBuf.overrun(_len))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      tmpBuf.skip(_len);

      _len <<= tmpBuf;
      if (_len == 0 || tmpBuf.overrun(_len))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      tmpBuf.skip(_len);

      CORBA::TypeCode_var discrimTCp;

      if (pd_tck == CORBA::tk_union)
	{
	  CORBA::TypeCode unExpandedTC(CORBA::tk_null,0);
	  unExpandedTC <<= tmpBuf;
	  discrimTCp = unExpandedTC.NP_aliasExpand();

	  CORBA::Long def;
	  def <<= tmpBuf;
	}

      CORBA::ULong count;
      count <<= tmpBuf;

      if ((count == 0 && pd_tck != CORBA::tk_except) || 
	  tmpBuf.overrun(count*sizeof(CORBA::ULong))) 
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      else if (index >= count) throw CORBA::TypeCode::Bounds();
      
      
      for(CORBA::ULong i=0; i <= index; i++)
	{
	  if (pd_tck == CORBA::tk_union)
	    {
	      CORBA::ULong discrimLen = 0;
	      CORBA::Char* label = 
	      tcParseEngine::getUnionLabel(discrimTCp->pd_tck,tmpBuf,
					   discrimLen);
	      delete[] label;
	    }

	  _len <<= tmpBuf;
	  
	  if (_len > tmpBuf.unRead() || _len == 0)
	    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
	  else if (i == index) break;
	  else tmpBuf.skip(_len);

	  if (pd_tck != CORBA::tk_enum)
	    {
	      CORBA::TypeCode tmpTC(CORBA::tk_null,0);
	      tmpTC <<= tmpBuf;
	    }
	}
	
      const char* p1 = (const char*) tmpBuf.data();
      return p1;
    }
 else
   {
     // Operation not valid on this TypeCode.
     throw CORBA::TypeCode::BadKind();
     return 0;
   }
}



CORBA::TypeCode_ptr 
CORBA::TypeCode::member_type(CORBA::ULong index) const
{
  if (pd_tck == CORBA::tk_struct || pd_tck == CORBA::tk_except || 
      pd_tck == CORBA::tk_union)
    {
      CORBA::ULong _len;

      MemBufferedStream tmpBuf(pd_param,1);
      tmpBuf.skip(1);

      _len <<= tmpBuf;
      if (_len == 0 || tmpBuf.overrun(_len))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      tmpBuf.skip(_len);

      _len <<= tmpBuf;
      if (_len == 0 || tmpBuf.overrun(_len))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      tmpBuf.skip(_len);

      CORBA::TypeCode_var discrimTCp;

      if (pd_tck == CORBA::tk_union)
	{
	  CORBA::TypeCode unExpandedTC(CORBA::tk_null,0);
	  unExpandedTC <<= tmpBuf;
	  discrimTCp = unExpandedTC.NP_aliasExpand();
	  
	  CORBA::Long def;
	  def <<= tmpBuf;
	}

      CORBA::ULong count;
      count <<= tmpBuf;

      if ((count == 0 && pd_tck != CORBA::tk_except) || 
	  tmpBuf.overrun(count*2*sizeof(CORBA::ULong))) 
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      else if (index >= count) throw CORBA::TypeCode::Bounds();

      CORBA::TypeCode_ptr rcTC = new CORBA::TypeCode(CORBA::tk_null,0);

      for(CORBA::ULong i=0; i <= index; i++)
	{
	  CORBA::release(rcTC);
	  if (pd_tck == CORBA::tk_union)
	    {
	      CORBA::ULong discrimLen = 0;
	      CORBA::Char* label =  tcParseEngine::getUnionLabel(
					 discrimTCp->pd_tck,tmpBuf,discrimLen);
	      delete[] label;
	    }

	  _len <<= tmpBuf;
	  if (_len > tmpBuf.unRead() || _len == 0)
	    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);

	  tmpBuf.skip(_len);

	  rcTC = new CORBA::TypeCode(CORBA::tk_null,0);      
	  try
	    {
	      *rcTC <<= tmpBuf;
	    }
	  catch(...)
	    {
	      CORBA::release(rcTC);
	      throw;
	    }
	}

      return rcTC;
    }
  else
    {
      // Operation not valid on this TypeCode.
      throw CORBA::TypeCode::BadKind();
      return 0;
    }
}



CORBA::Any*
CORBA::TypeCode::member_label(CORBA::ULong index) const
{
  if (pd_tck == CORBA::tk_union)
    {
      CORBA::ULong _len;
      
      MemBufferedStream tmpBuf(pd_param,1);
      tmpBuf.skip(1);

      _len <<= tmpBuf;
      if (_len == 0 || tmpBuf.overrun(_len))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      tmpBuf.skip(_len);

      _len <<= tmpBuf;
      if (_len == 0 || tmpBuf.overrun(_len))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      tmpBuf.skip(_len);
      
      CORBA::TypeCode unExpandedTC(CORBA::tk_null,0);

      unExpandedTC <<= tmpBuf;
      CORBA::TypeCode_var discrimTCp = unExpandedTC.NP_aliasExpand();

      CORBA::Long def;
      def <<= tmpBuf;
    
      CORBA::ULong count;
      count <<= tmpBuf;

      if (count == 0 || tmpBuf.overrun(count*2*sizeof(CORBA::ULong))) 
	  throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      else if (index >= count) throw CORBA::TypeCode::Bounds();

      CORBA::Char* label = 0;
      CORBA::Any* retval = 0;

      if (index == (CORBA::ULong) def)
	{
	  retval = new CORBA::Any;
	  CORBA::Octet _oct = 0;
	  try
	    {
	      *retval <<= CORBA::Any::from_octet(_oct);
	    }
	  catch(...)
	    {
	      delete retval;
	      throw;
	    }
	}
      else
	{
	  for(CORBA::ULong i=0; i <= index; i++)
	    {
	      CORBA::ULong discrimLen = 0;
	      label = tcParseEngine::getUnionLabel(discrimTCp->pd_tck,tmpBuf,
						   discrimLen);
	      if (index == i) break;
	      else delete[] label;
	      
	      _len <<= tmpBuf;
	      
	      if (_len > tmpBuf.unRead() || _len == 0)
		throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
	      else tmpBuf.skip(_len);
	      
	      CORBA::TypeCode tmpTC(CORBA::tk_null,0);
	      tmpTC <<= tmpBuf;
	    }

	  retval = 
	    new CORBA::Any(CORBA::TypeCode::_duplicate(discrimTCp),label,1,1);
	}
      return retval;
    }
  else
    {
      // Operation not valid on this TypeCode.
      throw CORBA::TypeCode::BadKind();
      return 0;
    }
}



CORBA::TypeCode_ptr 
CORBA::TypeCode::discriminator_type() const
{
  if (pd_tck == CORBA::tk_union)
    {
      CORBA::ULong _len;

      MemBufferedStream tmpBuf(pd_param,1);
      tmpBuf.skip(1);

      _len <<= tmpBuf;
      if (_len == 0 || tmpBuf.overrun(_len))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      tmpBuf.skip(_len);

      _len <<= tmpBuf;
      if (_len == 0 || tmpBuf.overrun(_len))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      tmpBuf.skip(_len);

      CORBA::TypeCode_ptr TCp = new CORBA::TypeCode(CORBA::tk_null,0);
      
      try
	{
	  *TCp <<= tmpBuf;
	}
      catch(...)
	{
	  CORBA::release(TCp);
	  throw;
	}

      return TCp;
    }
  else
    {
      // Operation not valid on this TypeCode.
      throw CORBA::TypeCode::BadKind();
      return 0;
    }
}   
    


CORBA::Long
CORBA::TypeCode::default_index() const
{
  if (pd_tck == CORBA::tk_union)
    {
      CORBA::ULong _len;

      MemBufferedStream tmpBuf(pd_param,1);
      tmpBuf.skip(1);

      _len <<= tmpBuf;
      if (_len == 0 || tmpBuf.overrun(_len))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      tmpBuf.skip(_len);

      _len <<= tmpBuf;
      if (_len == 0 || tmpBuf.overrun(_len))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      tmpBuf.skip(_len);


      CORBA::TypeCode a(CORBA::tk_null,0);
      a <<= tmpBuf;

      CORBA::Long def;
      def <<= tmpBuf;

      if (def > 0 && (4+(def*(1+2*sizeof(CORBA::ULong))) > tmpBuf.unRead()))
	throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);

      return def; 
    }
  else
    {
      // Operation not valid on this TypeCode.
      throw CORBA::TypeCode::BadKind();
      return 0;
    }
}   



CORBA::ULong
CORBA::TypeCode::length() const
{
  if (pd_tck == CORBA::tk_string)
    {
      return pd_maxLen;
    }
  else if (pd_tck == CORBA::tk_sequence || pd_tck == CORBA::tk_array)
    {
      MemBufferedStream tmpBuf(pd_param,1);
      tmpBuf.skip(1);

      CORBA::TypeCode tmpTC(CORBA::tk_null,0);
      CORBA::ULong len;

      tmpTC <<= tmpBuf;
      len <<= tmpBuf;
      
      return len;
    }
  else 
    {
      // Operation not valid on this TypeCode.
      throw CORBA::TypeCode::BadKind();
      return 0;
    }
}   



CORBA::TypeCode_ptr 
CORBA::TypeCode::content_type() const
{
  if (pd_tck == CORBA::tk_sequence || pd_tck == CORBA::tk_array || 
      pd_tck == CORBA::tk_alias)
    {
      MemBufferedStream tmpBuf(pd_param,1);
      tmpBuf.skip(1);

      if (pd_tck == CORBA::tk_alias)
	{
	  CORBA::ULong idLen;

	  idLen <<= tmpBuf;
	  if (idLen == 0 || tmpBuf.overrun(idLen))
	    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
	  tmpBuf.skip(idLen);

	  idLen <<= tmpBuf;
	  if (idLen == 0 || tmpBuf.overrun(idLen))
	    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
	  tmpBuf.skip(idLen,omni::ALIGN_4);
	}

      CORBA::TypeCode_ptr contentTC = new CORBA::TypeCode(CORBA::tk_null,0);
      try
	{
	  *contentTC <<= tmpBuf;
	}
      catch(...)
	{
	  CORBA::release(contentTC);
	  throw;
	}

      return contentTC;
    }
  else 
    {
      // Operation not valid on this TypeCode.
      throw CORBA::TypeCode::BadKind();
      return 0;
    }
}   



CORBA::Long 
CORBA::TypeCode::param_count() const
{
  if (pd_tck < CORBA::tk_objref) return 0;

  switch(pd_tck)
    {
    case CORBA::tk_objref:
    case CORBA::tk_string:
      return 1;
      
    case CORBA::tk_alias:
    case CORBA::tk_sequence:
    case CORBA::tk_array:
      return 2;

    case CORBA::tk_enum:
      return this->member_count() + 1;

    case CORBA::tk_struct:
    case CORBA::tk_except:
      return (2*this->member_count()) + 1;

    case CORBA::tk_union:
      return (3*this->member_count()) + 2;

    default:
      throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
      return 0;
    }


}



CORBA::Any* 
CORBA::TypeCode::parameter(CORBA::Long index) const
{
  CORBA::Any* rv;

  if (index < 0 || pd_tck < CORBA::tk_objref) throw CORBA::TypeCode::Bounds();
  else if (index == 0 && (pd_tck == CORBA::tk_union || 
			  pd_tck == CORBA::tk_alias ||
			  pd_tck == CORBA::tk_enum || 
			  pd_tck == CORBA::tk_struct ||
			  pd_tck == CORBA::tk_except))
    {
      const char* p1 = this->name();
      rv = new CORBA::Any;
      try
	{
	  *rv <<= p1;
	}
      catch(...)
	{
	  delete rv;
	  throw;
	}
      return rv;
    }
  else if (index == 1 && (pd_tck == CORBA::tk_union || 
			           pd_tck == CORBA::tk_alias))
    {
      CORBA::TypeCode_var TCp;
      if (pd_tck == CORBA::tk_union) TCp = this->discriminator_type();
      else TCp = this->content_type();
      rv = new CORBA::Any;
      try
	{
	  *rv <<= (CORBA::TypeCode_ptr) TCp;
	}
      catch(...)
	{
	  delete rv;
	  throw;
	}
      return rv;
    }

  switch(pd_tck) 
    {            
    case CORBA::tk_string:
      {
	if (index == 0)
	  {
	    rv = new CORBA::Any;
	    try
	      {
		*rv <<= this->pd_maxLen;
	      }
	    catch(...)
	      {
		delete rv;
		throw;
	      }
	    return rv;
	  }
	break;
      }

    case CORBA::tk_sequence:
    case CORBA::tk_array:
      {
	if (index == 0)
	  {
	    CORBA::TypeCode_var TCp = this->content_type();
	    rv = new CORBA::Any;
	    try
	      {
		*rv <<= (CORBA::TypeCode_ptr) TCp;
	      }
	    catch(...)
	      {
		delete rv;
		throw;
	      }

	    return rv;
	  }
	else if (index == 1)
	  {
	    CORBA::ULong _len = this->length();
	    rv = new CORBA::Any;
	    try
	      {
		*rv <<= _len;
	      }
	    catch(...)
	      {
		delete rv;
		throw;
	      }

	    return rv;
	  }
	break;
      }

    case CORBA::tk_objref:
      {
	if (index == 0)
	  {
	    const char* p1 = this->id();
	    rv = new CORBA::Any;
	    try
	      {
		*rv <<= p1;
	      }
	    catch(...)
	      {
		delete rv;
		throw;
	      }

	    return rv;
	  }
	break;
      }

    case CORBA::tk_enum:
      {
	const char* p1 = this->member_name(index - 1);
	rv = new CORBA::Any;
	try
	  {
	    *rv <<= p1;
	  }
	catch(...)
	  {
	    delete rv;
	    throw;
	  }
	return rv;
      }

    case CORBA::tk_struct:
    case CORBA::tk_except:
      {
	if(index%2 == 1)
	  {
	    const char* p1 = this->member_name((index+1)/2);
	    rv = new CORBA::Any;
	    try
	      {
		*rv <<= p1;
	      }
	    catch(...)
	      {
		delete rv;
		throw;
	      }

	    return rv;
	  }
	else
	  {
	    CORBA::TypeCode_var TCp = this->member_type(index/2);
	    rv = new CORBA::Any;
	    try
	      {
		*rv <<= (CORBA::TypeCode_ptr) TCp;
	      }
	    catch(...)
	      {
		delete rv;
		throw;
	      }

	    return rv;
	  }
      }

    case CORBA::tk_union:
      {
	if (index%3 == 2)
	  {
	    rv = this->member_label((index+1)/3);
	    return rv;
	  }
	else if (index%3 == 1)
	  {
	    const char* p1 = this->member_name(index/3);
	    rv = new Any;
	    try
	      {
		*rv <<= p1;
	      }
	    catch(...)
	      {
		delete rv;
		throw;
	      }

	    return rv;
	  }
	else
	  {
	    CORBA::TypeCode_var TCp = this->member_type((index-1)/3);
	    rv = new Any;
	    try
	      {
		*rv <<= (CORBA::TypeCode_ptr) TCp;
	      }
	    catch(...)
	      {
		delete rv;
		throw;
	      }

	    return rv;
	  }
      }
      
    default:
      break;
    }      

  throw CORBA::TypeCode::Bounds();
  return 0;
}



CORBA::TypeCode_ptr 
CORBA::TypeCode::_duplicate(CORBA::TypeCode_ptr t)
{
  if (t->NP_is_nil())
    return CORBA::TypeCode::_nil();
  else
    {
      CORBA::TypeCode_ptr tc = new CORBA::TypeCode(*t);
      return tc;
    }
}



CORBA::TypeCode_ptr
CORBA::TypeCode::_nil()
{
  if (!_nil_TypeCodeV) {
#if !defined(__DECCXX_VER) || __DECCXX_VER > 50590004
    _nil_TypeCodeV == new CORBA::_nil_TypeCode;
#else
    // Workaround for compiler bug in DEC cxx v5.5
    CORBA::_nil_TypeCode tmp;
    _nil_TypeCodeV = new CORBA::TypeCode(tmp);
#endif
  }
  return _nil_TypeCodeV;
}


/**************************************************************************/

CORBA::TypeCode_ptr 
CORBA::ORB::create_struct_tc(const char* id, const char* name, 
			const CORBA::StructMemberSeq& members) 
{
  return new CORBA::TypeCode(CORBA::tk_struct,id,name,members);
}


CORBA::TypeCode_ptr 
CORBA::ORB::create_union_tc(const char* id, const char* name, 
			    CORBA::TypeCode_ptr discriminator_type, 
			    const CORBA::UnionMemberSeq& members) 
{
  return new CORBA::TypeCode(id,name,discriminator_type,members);
}

    
CORBA::TypeCode_ptr 
CORBA::ORB::create_enum_tc(const char* id, const char* name, 
			   const CORBA::EnumMemberSeq& members) 
{
  return new CORBA::TypeCode(id,name,members);
}


CORBA::TypeCode_ptr 
CORBA::ORB::create_alias_tc(const char* id, const char* name, 
			    CORBA::TypeCode_ptr original_type) 
{
  return new CORBA::TypeCode(id,name,*original_type);
}


CORBA::TypeCode_ptr 
CORBA::ORB::create_exception_tc(const char* id, const char* name, 
				const CORBA::StructMemberSeq& members) 
{
  return new CORBA::TypeCode(CORBA::tk_except,id,name,members);
}


CORBA::TypeCode_ptr 
CORBA::ORB::create_interface_tc(const char* id, const char* name) 
{
  return new CORBA::TypeCode(id,name);
}


CORBA::TypeCode_ptr 
CORBA::ORB::create_string_tc(CORBA::ULong bound) 
{
  return new CORBA::TypeCode(CORBA::tk_string,bound);
}


CORBA::TypeCode_ptr 
CORBA::ORB::create_sequence_tc(CORBA::ULong bound, 
			       CORBA::TypeCode_ptr element_type)
{
  return new CORBA::TypeCode(CORBA::tk_sequence,bound,*element_type);
}


CORBA::TypeCode_ptr 
CORBA::ORB::create_array_tc(CORBA::ULong length, 
			    CORBA::TypeCode_ptr element_type) 
{
  return new CORBA::TypeCode(CORBA::tk_array,length,*element_type);
}


CORBA::TypeCode_ptr
CORBA::ORB::create_recursive_sequence_tc(CORBA::ULong bound, 
					 CORBA::ULong offset) 
{
  // Not implemented yet
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
  return 0;
}

/**************************************************************************/

CORBA::TypeCode_member::TypeCode_member() 
{ 
  _ptr = CORBA::TypeCode::_nil();
}

CORBA::TypeCode_member::TypeCode_member(CORBA::TypeCode_ptr p) 
{ 
  _ptr = p;
}


CORBA::TypeCode_member::TypeCode_member(const CORBA::TypeCode_member& p)
{
  if ((p._ptr)->NP_is_nil()) _ptr = CORBA::TypeCode::_nil(); 
  else _ptr = new TypeCode(*(p._ptr));
}

CORBA::TypeCode_member::~TypeCode_member()
{
  CORBA::release(_ptr);
}

    
CORBA::TypeCode_member& 
CORBA::TypeCode_member::operator=(CORBA::TypeCode_ptr p)
{
  CORBA::release(_ptr);
  _ptr = p;
  return *this;
}

    
CORBA::TypeCode_member&
CORBA::TypeCode_member::operator=(const CORBA::TypeCode_member& p)
{
  if (this != &p) {
    CORBA::release(_ptr);
    if ((p._ptr)->NP_is_nil()) _ptr = CORBA::TypeCode::_nil(); 
    else _ptr = new TypeCode(*(p._ptr));
  }
  return *this;
}

CORBA::TypeCode_member&
CORBA::TypeCode_member::operator=(const CORBA::TypeCode_var& p)
{
  CORBA::release(_ptr);
  if ((p.pd_TC)->NP_is_nil()) _ptr = CORBA::TypeCode::_nil();
  else _ptr = new TypeCode(*(p.pd_TC));
  return *this;
}

void
CORBA::TypeCode_member::operator>>=(NetBufferedStream& s) const
{ 
  *_ptr >>= s;
}

void
CORBA::TypeCode_member::operator<<=(NetBufferedStream& s)
{
  TypeCode_ptr _result = new TypeCode(tk_null,0);
  *_result <<= s;
  CORBA::release(_ptr);
  _ptr = _result;
}

void
CORBA::TypeCode_member::operator>>=(MemBufferedStream& s) const
{ 
  *_ptr >>= s;
}

void
CORBA::TypeCode_member::operator<<=(MemBufferedStream& s)
{
  TypeCode_ptr _result = new TypeCode(tk_null,0);
  *_result <<= s;
  CORBA::release(_ptr);
  _ptr = _result;
}
      
size_t
CORBA::TypeCode_member::NP_alignedSize(size_t initialoffset) const
{
  return _ptr->NP_alignedSize(initialoffset);
}

    
CORBA::TypeCode_ptr
CORBA::TypeCode_member::operator->() const
{ 
  return (TypeCode_ptr) _ptr;
}

CORBA::TypeCode_member::operator CORBA::TypeCode_ptr() const
{ 
  return _ptr;
}


/**************************************************************************/


// Following used by Interface Repository - and use IDLType:

size_t 
CORBA::StructMember::NP_alignedSize(size_t initialoffset) const 
{
  // Not implemented - used by IR
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
  return 0;      
}
    
void 
CORBA::StructMember::operator>>= (NetBufferedStream &) const 
{
  // Not implemented - used by IR
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
}

void 
CORBA::StructMember::operator<<= (NetBufferedStream &) 
{
  // Not implemented - used by IR
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
}

void 
CORBA::StructMember::operator>>= (MemBufferedStream &) const 
{
  // Not implemented - used by IR
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
}

void 
CORBA::StructMember::operator<<= (MemBufferedStream &) 
{      
  // Not implemented - used by IR
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
}



size_t 
CORBA::UnionMember::NP_alignedSize(size_t initialoffset) const 
{
  // Not implemented - used by IR
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
  return 0;      
}
    
void 
CORBA::UnionMember::operator>>= (NetBufferedStream &) const 
{
  // Not implemented - used by IR
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
}

void 
CORBA::UnionMember::operator<<= (NetBufferedStream &) 
{
  // Not implemented - used by IR
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
}

void 
CORBA::UnionMember::operator>>= (MemBufferedStream &) const 
{
  // Not implemented - used by IR
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
}

void 
CORBA::UnionMember::operator<<= (MemBufferedStream &) 
{      
  // Not implemented - used by IR
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
}


CORBA::Boolean 
CORBA::TypeCode::NP_is_nil() const 
{ 
  return 0; 
}


CORBA::Boolean 
CORBA::_nil_TypeCode::NP_is_nil() const 
{ 
  return 1; 
}


/**************************************************************************/

CORBA::IDLType::~IDLType()
{
}

CORBA::TypeCode_ptr
CORBA::IDLType::type()
{
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

CORBA::IDLType_ptr
CORBA::IDLType::_duplicate(CORBA::IDLType_ptr obj)
{
  if (CORBA::is_nil(obj))
    return IDLType::_nil();
  else {
    throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO); 
#ifdef NEED_DUMMY_RETURN
   return 0;
#endif
  }
}
    
CORBA::IDLType_ptr
CORBA::IDLType::_narrow(CORBA::Object_ptr obj)
{
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO); 
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}
    
CORBA::IDLType_ptr
CORBA::IDLType::_nil() 
{
  if (!_nil_IDLTypeV) {
    _nil_IDLTypeV = new _nil_IDLType;
#ifndef HAS_Cplusplus_Namespace
    // deprecated. Left here for backward compatiability.
    CORBA::__nil_IDLType = _nil_IDLTypeV;
#endif
  }
  return _nil_IDLTypeV;
}

size_t
CORBA::IDLType::NP_alignedSize(CORBA::IDLType_ptr obj,size_t initialoffset)
{
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
  return 0;      
#endif
}

void
CORBA::IDLType::marshalObjRef(CORBA::IDLType_ptr obj,NetBufferedStream &s)
{
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
}

CORBA::IDLType_ptr
CORBA::IDLType::unmarshalObjRef(NetBufferedStream &s)
{
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

void
CORBA::IDLType::marshalObjRef(IDLType_ptr obj,MemBufferedStream &s)
{
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
}

CORBA::IDLType_ptr
CORBA::IDLType::unmarshalObjRef(MemBufferedStream &s)
{
  throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

CORBA::_nil_IDLType::_nil_IDLType() 
     : omniObject(omniObject::nilObjectManager())
{ 
  this->PR_setobj(0);
}

CORBA::_nil_IDLType::~_nil_IDLType()
{
}
    
CORBA::TypeCode_ptr
CORBA::
_nil_IDLType::type ()
{
  throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
  // never reach here! Dummy return to keep some compilers happy.
  CORBA::TypeCode_ptr _result= 0;
  return _result;
#endif
}


/**************************************************************************/


// TypeCode Constants:



static CORBA::TypeCode _01RL__tc_null__(CORBA::tk_null,0);
static CORBA::TypeCode _01RL__tc_void__(CORBA::tk_void,0);
static CORBA::TypeCode _01RL__tc_short__(CORBA::tk_short,0);
static CORBA::TypeCode _01RL__tc_long__(CORBA::tk_long,0);
static CORBA::TypeCode _01RL__tc_ushort__(CORBA::tk_ushort,0);
static CORBA::TypeCode _01RL__tc_ulong__(CORBA::tk_ulong,0);
static CORBA::TypeCode _01RL__tc_float__(CORBA::tk_float,0);
static CORBA::TypeCode _01RL__tc_double__(CORBA::tk_double,0);
static CORBA::TypeCode _01RL__tc_boolean__(CORBA::tk_boolean,0);
static CORBA::TypeCode _01RL__tc_char__(CORBA::tk_char,0);
static CORBA::TypeCode _01RL__tc_octet__(CORBA::tk_octet,0);
static CORBA::TypeCode _01RL__tc_any__(CORBA::tk_any,0);
static CORBA::TypeCode _01RL__tc_TypeCode__(CORBA::tk_TypeCode,0);
static CORBA::TypeCode _01RL__tc_Principal__(CORBA::tk_Principal,0);
static CORBA::TypeCode _01RL__tc_Object__("IDL:CORBA/Object:1.0","Object");
static CORBA::TypeCode _01RL__tc_string__(CORBA::tk_string,0);
				   
#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constants external linkage otherwise. Its a bug.
namespace CORBA {

extern const TypeCode_ptr         _tc_null = &_01RL__tc_null__;
extern const TypeCode_ptr         _tc_void = &_01RL__tc_void__;
extern const TypeCode_ptr         _tc_short = &_01RL__tc_short__;
extern const TypeCode_ptr         _tc_long = &_01RL__tc_long__;
extern const TypeCode_ptr         _tc_ushort = &_01RL__tc_ushort__;
extern const TypeCode_ptr         _tc_ulong = &_01RL__tc_ulong__;
extern const TypeCode_ptr         _tc_float = &_01RL__tc_float__;
extern const TypeCode_ptr         _tc_double = &_01RL__tc_double__;
extern const TypeCode_ptr         _tc_boolean = &_01RL__tc_boolean__;
extern const TypeCode_ptr         _tc_char = &_01RL__tc_char__;
extern const TypeCode_ptr         _tc_octet = &_01RL__tc_octet__;
extern const TypeCode_ptr         _tc_any = &_01RL__tc_any__;
extern const TypeCode_ptr         _tc_TypeCode = &_01RL__tc_TypeCode__;
extern const TypeCode_ptr         _tc_Principal = &_01RL__tc_Principal__;
extern const TypeCode_ptr         _tc_Object = &_01RL__tc_Object__;
extern const TypeCode_ptr         _tc_string = &_01RL__tc_string__;

}
#else
const CORBA::TypeCode_ptr         CORBA::_tc_null = &_01RL__tc_null__;
const CORBA::TypeCode_ptr         CORBA::_tc_void = &_01RL__tc_void__;
const CORBA::TypeCode_ptr         CORBA::_tc_short = &_01RL__tc_short__;
const CORBA::TypeCode_ptr         CORBA::_tc_long = &_01RL__tc_long__;
const CORBA::TypeCode_ptr         CORBA::_tc_ushort = &_01RL__tc_ushort__;
const CORBA::TypeCode_ptr         CORBA::_tc_ulong = &_01RL__tc_ulong__;
const CORBA::TypeCode_ptr         CORBA::_tc_float = &_01RL__tc_float__;
const CORBA::TypeCode_ptr         CORBA::_tc_double = &_01RL__tc_double__;
const CORBA::TypeCode_ptr         CORBA::_tc_boolean = &_01RL__tc_boolean__;
const CORBA::TypeCode_ptr         CORBA::_tc_char = &_01RL__tc_char__;
const CORBA::TypeCode_ptr         CORBA::_tc_octet = &_01RL__tc_octet__;
const CORBA::TypeCode_ptr         CORBA::_tc_any = &_01RL__tc_any__;
const CORBA::TypeCode_ptr         CORBA::_tc_TypeCode = &_01RL__tc_TypeCode__;
const CORBA::TypeCode_ptr         CORBA::_tc_Principal = 
                                                       &_01RL__tc_Principal__;
const CORBA::TypeCode_ptr         CORBA::_tc_Object = &_01RL__tc_Object__;
const CORBA::TypeCode_ptr         CORBA::_tc_string = &_01RL__tc_string__;
#endif
